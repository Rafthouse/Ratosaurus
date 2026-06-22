#include "OversamplingEngine.h"
#include <algorithm>
#include <cmath>

namespace rato
{

void OversamplingEngine::prepare(double sampleRate, int maximumBlockSize,
                                 int numChannels, int initialMode)
{
    currentSampleRate = sampleRate;
    activeMode = juce::jlimit(0, 3, initialMode);
    targetMode = activeMode;
    transitionLength = std::max(1, static_cast<int>(std::round(sampleRate * 0.02)));
    transitionRemaining = 0;

    maximumLatency = 0;
    modeLatencies.fill(0);

    for (int index = 0; index < 3; ++index)
    {
        const auto factorPower = static_cast<size_t>(index + 1); // 2x, 4x, 8x
        auto instance = std::make_unique<Oversampler>(
            static_cast<size_t>(std::max(1, numChannels)),
            factorPower,
            Oversampler::filterHalfBandFIREquiripple,
            true,
            true);

        instance->initProcessing(static_cast<size_t>(maximumBlockSize));
        instance->reset();

        const int latency = static_cast<int>(std::round(instance->getLatencyInSamples()));
        modeLatencies[static_cast<size_t>(index + 1)] = latency;
        maximumLatency = std::max(maximumLatency, latency);
        oversamplers[static_cast<size_t>(index)] = std::move(instance);
    }

    const int delayBufferSize = std::max(1, maximumLatency + 1);
    for (auto& state : delayStates)
    {
        state.buffer.setSize(std::max(1, numChannels), delayBufferSize,
                             false, true, false);
        state.buffer.clear();
        state.writePosition = 0;
    }

    oldScratch.setSize(std::max(1, numChannels), std::max(1, maximumBlockSize),
                       false, true, false);
    newScratch.setSize(std::max(1, numChannels), std::max(1, maximumBlockSize),
                       false, true, false);
}

void OversamplingEngine::reset()
{
    for (auto& instance : oversamplers)
        if (instance != nullptr)
            instance->reset();

    for (auto& state : delayStates)
    {
        state.buffer.clear();
        state.writePosition = 0;
    }

    oldScratch.clear();
    newScratch.clear();
    transitionRemaining = 0;
    targetMode = activeMode;
}

void OversamplingEngine::beginTransition(int newMode)
{
    targetMode = juce::jlimit(0, 3, newMode);
    if (targetMode == activeMode)
        return;

    resetPath(targetMode);
    transitionRemaining = transitionLength;
}

void OversamplingEngine::resetPath(int mode)
{
    if (mode > 0)
        if (auto& instance = oversamplers[static_cast<size_t>(mode - 1)])
            instance->reset();

    auto& delay = delayStates[static_cast<size_t>(mode)];
    delay.buffer.clear();
    delay.writePosition = 0;
}

void OversamplingEngine::processPath(juce::dsp::AudioBlock<float> block,
                                     HarmonicsEngine& harmonics,
                                     int mode)
{
    if (mode == 0)
    {
        juce::dsp::ProcessContextReplacing<float> context(block);
        harmonics.process(context);
    }
    else
    {
        auto& instance = oversamplers[static_cast<size_t>(mode - 1)];
        auto upsampled = instance->processSamplesUp(block);
        juce::dsp::ProcessContextReplacing<float> context(upsampled);
        harmonics.process(context);
        instance->processSamplesDown(block);
    }

    applyLatencyCompensation(block, mode);
}

void OversamplingEngine::applyLatencyCompensation(juce::dsp::AudioBlock<float> block,
                                                   int mode)
{
    const int delaySamples = maximumLatency - modeLatencies[static_cast<size_t>(mode)];
    if (delaySamples <= 0)
        return;

    auto& state = delayStates[static_cast<size_t>(mode)];
    const int bufferSize = state.buffer.getNumSamples();
    const int channels = static_cast<int>(block.getNumChannels());
    const int samples = static_cast<int>(block.getNumSamples());

    for (int sample = 0; sample < samples; ++sample)
    {
        int readPosition = state.writePosition - delaySamples;
        if (readPosition < 0)
            readPosition += bufferSize;

        for (int channel = 0; channel < channels; ++channel)
        {
            auto* data = block.getChannelPointer(static_cast<size_t>(channel));
            const float input = data[sample];
            data[sample] = state.buffer.getSample(channel, readPosition);
            state.buffer.setSample(channel, state.writePosition, input);
        }

        state.writePosition = (state.writePosition + 1) % bufferSize;
    }
}

void OversamplingEngine::process(juce::AudioBuffer<float>& buffer,
                                 HarmonicsEngine& harmonics,
                                 int requestedMode)
{
    requestedMode = juce::jlimit(0, 3, requestedMode);
    const auto inputBlock = juce::dsp::AudioBlock<float>(buffer);

    if (transitionRemaining <= 0 && requestedMode != activeMode)
        beginTransition(requestedMode);

    if (transitionRemaining <= 0)
    {
        processPath(inputBlock, harmonics, activeMode);
        return;
    }

    const auto samples = inputBlock.getNumSamples();
    auto oldBlock = juce::dsp::AudioBlock<float>(oldScratch).getSubBlock(0, samples);
    auto newBlock = juce::dsp::AudioBlock<float>(newScratch).getSubBlock(0, samples);
    oldBlock.copyFrom(inputBlock);
    newBlock.copyFrom(inputBlock);

    processPath(oldBlock, harmonics, activeMode);
    processPath(newBlock, harmonics, targetMode);

    for (size_t sample = 0; sample < samples; ++sample)
    {
        const float mix = transitionRemaining > 0
            ? 1.0f - static_cast<float>(transitionRemaining)
                       / static_cast<float>(transitionLength)
            : 1.0f;

        for (size_t channel = 0; channel < inputBlock.getNumChannels(); ++channel)
        {
            const auto channelIndex = static_cast<int>(channel);
            const auto sampleIndex = static_cast<int>(sample);
            const float oldValue = oldBlock.getSample(channelIndex, sampleIndex);
            const float newValue = newBlock.getSample(channelIndex, sampleIndex);
            inputBlock.setSample(channelIndex, sampleIndex,
                                 oldValue + (newValue - oldValue) * mix);
        }

        if (transitionRemaining > 0)
            --transitionRemaining;
    }

    if (transitionRemaining <= 0)
        activeMode = targetMode;
}

} // namespace rato

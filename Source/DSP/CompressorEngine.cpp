#include "CompressorEngine.h"

namespace rato
{

void CompressorEngine::prepare(double sampleRate, int /*maxBlockSize*/, int /*numChannels*/)
{
    peakDetector.prepare(sampleRate);
    rmsDetector.prepare(sampleRate);
    characterEngine.prepare(sampleRate);
    currentGainReductionDb = 0.0f;
}

void CompressorEngine::reset()
{
    peakDetector.reset();
    rmsDetector.reset();
    characterEngine.reset();
    currentGainReductionDb = 0.0f;
}

void CompressorEngine::process(juce::AudioBuffer<float>& buffer)
{
    const auto numSamples  = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float maxAbsolute = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            maxAbsolute = std::max(maxAbsolute, std::abs(buffer.getSample(ch, sample)));

        const float envelopeLevel = detectLevel(maxAbsolute);

        const float levelDb = juce::Decibels::gainToDecibels(envelopeLevel, -96.0f);

        const float rawGrDb = gainComputer.computeGainReduction(levelDb);

        const float shapedGrDb = characterEngine.process(maxAbsolute, rawGrDb);

        currentGainReductionDb = shapedGrDb;

        const float gainLinear = juce::Decibels::decibelsToGain(shapedGrDb);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float in = buffer.getSample(ch, sample);
            buffer.setSample(ch, sample, in * gainLinear);
        }
    }
}

void CompressorEngine::setThreshold(float dB)
{
    gainComputer.setThreshold(dB);
}

void CompressorEngine::setRatio(float ratio)
{
    gainComputer.setRatio(ratio);
}

void CompressorEngine::setAttackMs(float ms)
{
    peakDetector.setAttackMs(ms);
    rmsDetector.setAttackMs(ms);
}

void CompressorEngine::setReleaseMs(float ms)
{
    peakDetector.setReleaseMs(ms);
    rmsDetector.setReleaseMs(ms);
}

void CompressorEngine::setDetectorMode(DetectorMode mode)
{
    detectorMode = mode;
}

void CompressorEngine::setKneeMode(KneeMode mode)
{
    switch (mode)
    {
        case KneeMode::Hard:   gainComputer.setKneeWidth(0.0f);  break;
        case KneeMode::Medium: gainComputer.setKneeWidth(6.0f);  break;
        case KneeMode::Soft:   gainComputer.setKneeWidth(12.0f); break;
    }
}

void CompressorEngine::setCharacterMode(CharacterMode mode)
{
    characterEngine.setMode(mode);
}

float CompressorEngine::detectLevel(float inputAbsolute)
{
    switch (detectorMode)
    {
        case DetectorMode::Peak: return peakDetector.processSample(inputAbsolute);
        case DetectorMode::RMS:  return rmsDetector.processSample(inputAbsolute);
    }

    return peakDetector.processSample(inputAbsolute);
}

} // namespace rato

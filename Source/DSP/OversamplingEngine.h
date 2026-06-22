#pragma once

#include "HarmonicsEngine.h"
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <memory>

namespace rato
{

/**
    Selectable FIR oversampling wrapper for the stateless HarmonicsEngine.

    All modes report the 8x path latency. Lower factors and Off are delayed to
    the same value, so changing the APVTS choice never invalidates host PDC.
    Mode changes are crossfaded over 20 ms using preallocated scratch buffers.
*/
class OversamplingEngine
{
public:
    OversamplingEngine() = default;

    void prepare(double sampleRate, int maximumBlockSize, int numChannels,
                 int initialMode);
    void reset();

    void process(juce::AudioBuffer<float>& buffer,
                 HarmonicsEngine& harmonics,
                 int requestedMode);

    int getLatencySamples() const noexcept { return maximumLatency; }

private:
    using Oversampler = juce::dsp::Oversampling<float>;

    struct DelayState
    {
        juce::AudioBuffer<float> buffer;
        int writePosition = 0;
    };

    std::array<std::unique_ptr<Oversampler>, 3> oversamplers;
    std::array<int, 4> modeLatencies{};
    std::array<DelayState, 4> delayStates;
    juce::AudioBuffer<float> oldScratch;
    juce::AudioBuffer<float> newScratch;

    double currentSampleRate = 44100.0;
    int maximumLatency = 0;
    int activeMode = 2; // 0=Off, 1=2x, 2=4x, 3=8x
    int targetMode = 2;
    int transitionLength = 1;
    int transitionRemaining = 0;

    void beginTransition(int newMode);
    void resetPath(int mode);
    void processPath(juce::dsp::AudioBlock<float> block,
                     HarmonicsEngine& harmonics,
                     int mode);
    void applyLatencyCompensation(juce::dsp::AudioBlock<float> block,
                                  int mode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OversamplingEngine)
};

} // namespace rato

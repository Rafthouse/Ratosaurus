#pragma once

#include <atomic>
#include <cmath>
#include <algorithm>
#include <juce_audio_basics/juce_audio_basics.h>

namespace rato
{

/**
    Peak meter with release smoothing.

    Real-time safe:
      • no allocations on the audio thread,
      • no locks,
      • level is published via std::atomic<float> for lock-free GUI reads,
      • no MessageManager / message-thread calls from processBlock.

    Attack is instantaneous (any sample > current peak overrides immediately).
    Release uses an exponential decay with a fixed time constant (~250 ms).
*/
class LevelMeter
{
public:
    void prepare(double sampleRate)
    {
        constexpr float releaseSeconds = 0.25f;
        const float fs = static_cast<float>(std::max(sampleRate, 1.0));
        releaseCoeff = std::exp(-1.0f / (releaseSeconds * fs));
        reset();
    }

    void reset()
    {
        internalLevel = 0.0f;
        publishedLevel.store(-100.0f, std::memory_order_relaxed);
    }

    void processBlock(const juce::AudioBuffer<float>& buffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples  = buffer.getNumSamples();

        float blockPeak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const auto* data = buffer.getReadPointer(ch);
            for (int i = 0; i < numSamples; ++i)
                blockPeak = std::max(blockPeak, std::abs(data[i]));
        }

        if (blockPeak > internalLevel)
        {
            internalLevel = blockPeak;
        }
        else
        {
            // Block-rate decay: per-sample coeff raised to numSamples.
            const float blockDecay = std::pow(releaseCoeff,
                                              static_cast<float>(numSamples));
            internalLevel *= blockDecay;
        }

        const float dbLevel = juce::Decibels::gainToDecibels(internalLevel, -100.0f);
        publishedLevel.store(dbLevel, std::memory_order_relaxed);
    }

    float getLevelDb() const noexcept
    {
        return publishedLevel.load(std::memory_order_relaxed);
    }

private:
    std::atomic<float> publishedLevel { -100.0f };
    float internalLevel = 0.0f;
    float releaseCoeff = 0.0f;
};

} // namespace rato

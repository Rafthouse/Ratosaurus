#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <cmath>

namespace rato
{

/** Audio-thread VU integration state. Contains no GUI objects. */
class VUMeterBallistics
{
public:
    void prepare(double newSampleRate) noexcept
    {
        sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
        alpha = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * timeConstantSeconds)));
        reset();
    }

    void reset() noexcept
    {
        smoothedPower = 0.0f;
        publishedVu.store(-60.0f, std::memory_order_relaxed);
    }

    void processBlock(const juce::AudioBuffer<float>& buffer,
                      float calibrationOffsetDb) noexcept
    {
        const int channels = buffer.getNumChannels();
        const int samples  = buffer.getNumSamples();
        if (channels <= 0 || samples <= 0)
            return;

        for (int sample = 0; sample < samples; ++sample)
        {
            float instantaneousPower = 0.0f;
            for (int channel = 0; channel < channels; ++channel)
            {
                const float value = buffer.getSample(channel, sample);
                instantaneousPower += value * value;
            }

            instantaneousPower /= static_cast<float>(channels);
            smoothedPower += alpha * (instantaneousPower - smoothedPower);
        }

        const float rms  = std::sqrt(juce::jmax(smoothedPower, 0.0f));
        const float dbfs = juce::jmax(-60.0f, juce::Decibels::gainToDecibels(rms, -60.0f));
        publishedVu.store(dbfs - calibrationOffsetDb, std::memory_order_relaxed);
    }

    float getCurrentVu() const noexcept
    {
        return publishedVu.load(std::memory_order_relaxed);
    }

    static float calibrationOffsetForChoice(int choice) noexcept
    {
        switch (choice)
        {
            case 1:  return -24.0f;
            case 2:  return -20.0f;
            case 0:
            default: return -18.0f;
        }
    }

private:
    static constexpr double timeConstantSeconds = 0.3;
    double sampleRate = 44100.0;
    float alpha = 0.0f;
    float smoothedPower = 0.0f;
    std::atomic<float> publishedVu { -60.0f };
};

} // namespace rato

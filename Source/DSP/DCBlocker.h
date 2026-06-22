#pragma once

#include <vector>
#include <algorithm>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace rato
{

/**
    1-pole high-pass DC blocker.
        y[n] = x[n] - x[n-1] + R · y[n-1]

    R is set such that the -3 dB corner is at ~5 Hz at the native sample rate.
    Per-channel state. Operates in-place on an AudioBuffer at native rate.
*/
class DCBlocker
{
public:
    void prepare(double sampleRate, int numChannels)
    {
        constexpr float cutoffHz = 5.0f;
        const float fs = static_cast<float>(sampleRate);
        R = 1.0f - 2.0f * juce::MathConstants<float>::pi * cutoffHz / std::max(fs, 1.0f);

        const auto channels = static_cast<size_t>(std::max(0, numChannels));
        x1.assign(channels, 0.0f);
        y1.assign(channels, 0.0f);
    }

    void reset()
    {
        std::fill(x1.begin(), x1.end(), 0.0f);
        std::fill(y1.begin(), y1.end(), 0.0f);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        const auto numChannels = std::min(buffer.getNumChannels(),
                                          static_cast<int>(x1.size()));
        const auto numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            float prevX = x1[static_cast<size_t>(ch)];
            float prevY = y1[static_cast<size_t>(ch)];

            for (int i = 0; i < numSamples; ++i)
            {
                const float in  = data[i];
                const float out = in - prevX + R * prevY;
                data[i] = out;
                prevX = in;
                prevY = out;
            }

            x1[static_cast<size_t>(ch)] = prevX;
            y1[static_cast<size_t>(ch)] = prevY;
        }
    }

private:
    float R = 0.995f;
    std::vector<float> x1;
    std::vector<float> y1;
};

} // namespace rato

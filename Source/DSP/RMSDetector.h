#pragma once

#include "EnvelopeDetector.h"
#include <vector>

namespace rato
{

class RMSDetector final : public EnvelopeDetector
{
public:
    void prepare(double sampleRate) override
    {
        EnvelopeDetector::prepare(sampleRate);

        windowSize = std::max(1, static_cast<int>(sampleRate * windowSeconds));
        circularBuffer.assign(static_cast<size_t>(windowSize), 0.0f);
        writeIndex = 0;
        runningSum = 0.0f;
    }

    void reset() override
    {
        EnvelopeDetector::reset();
        std::fill(circularBuffer.begin(), circularBuffer.end(), 0.0f);
        writeIndex = 0;
        runningSum = 0.0f;
    }

    float processSample(float inputAbsolute) override
    {
        const float inputSquared = inputAbsolute * inputAbsolute;

        runningSum -= circularBuffer[static_cast<size_t>(writeIndex)];
        circularBuffer[static_cast<size_t>(writeIndex)] = inputSquared;
        runningSum += inputSquared;

        writeIndex = (writeIndex + 1) % windowSize;

        if (runningSum < 0.0f)
            runningSum = 0.0f;

        const float rmsLevel = std::sqrt(runningSum / static_cast<float>(windowSize));

        if (rmsLevel > envelope)
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * rmsLevel;
        else
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * rmsLevel;

        return envelope;
    }

private:
    static constexpr double windowSeconds = 0.01;

    std::vector<float> circularBuffer;
    int windowSize = 0;
    int writeIndex = 0;
    float runningSum = 0.0f;
};

} // namespace rato

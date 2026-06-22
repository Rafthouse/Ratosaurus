#pragma once

#include <cmath>
#include <algorithm>

namespace rato
{

class GainComputer
{
public:
    void setThreshold(float dB)  { threshold = dB; }
    void setRatio(float r)       { ratio = std::max(1.0f, r); }
    void setKneeWidth(float dB)  { kneeWidth = std::max(0.0f, dB); }

    float getThreshold() const noexcept { return threshold; }
    float getRatio()     const noexcept { return ratio; }
    float getKneeWidth() const noexcept { return kneeWidth; }

    float computeGainReduction(float inputDb) const noexcept
    {
        if (ratio <= 1.0f)
            return 0.0f;

        const float slope = 1.0f - (1.0f / ratio);

        if (kneeWidth <= 0.0f)
            return computeHardKnee(inputDb, slope);

        return computeSoftKnee(inputDb, slope);
    }

private:
    float threshold = -20.0f;
    float ratio     = 4.0f;
    float kneeWidth = 0.0f;

    float computeHardKnee(float inputDb, float slope) const noexcept
    {
        if (inputDb <= threshold)
            return 0.0f;

        return -slope * (inputDb - threshold);
    }

    float computeSoftKnee(float inputDb, float slope) const noexcept
    {
        const float halfKnee = kneeWidth * 0.5f;
        const float lowerBound = threshold - halfKnee;
        const float upperBound = threshold + halfKnee;

        if (inputDb <= lowerBound)
            return 0.0f;

        if (inputDb >= upperBound)
            return -slope * (inputDb - threshold);

        const float x = inputDb - lowerBound;
        return -slope * (x * x) / (2.0f * kneeWidth);
    }
};

} // namespace rato

#pragma once

#include <cmath>
#include <algorithm>

namespace rato
{

class EnvelopeDetector
{
public:
    virtual ~EnvelopeDetector() = default;

    virtual void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        recalculateCoefficients();
    }

    void setAttackMs(float ms)
    {
        attackMs = std::max(0.01f, ms);
        recalculateCoefficients();
    }

    void setReleaseMs(float ms)
    {
        releaseMs = std::max(0.01f, ms);
        recalculateCoefficients();
    }

    virtual void reset()
    {
        envelope = 0.0f;
    }

    virtual float processSample(float inputAbsolute) = 0;

    float getEnvelope() const noexcept { return envelope; }

protected:
    float attackCoeff  = 0.0f;
    float releaseCoeff = 0.0f;
    float envelope     = 0.0f;
    double currentSampleRate = 44100.0;

    static float msToCoeff(float timeMs, double sampleRate)
    {
        if (timeMs <= 0.0f || sampleRate <= 0.0)
            return 0.0f;

        return std::exp(-1000.0f / (timeMs * static_cast<float>(sampleRate)));
    }

private:
    float attackMs  = 10.0f;
    float releaseMs = 100.0f;

    void recalculateCoefficients()
    {
        attackCoeff  = msToCoeff(attackMs, currentSampleRate);
        releaseCoeff = msToCoeff(releaseMs, currentSampleRate);
    }
};

} // namespace rato

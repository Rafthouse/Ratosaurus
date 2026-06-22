#pragma once

#include "EnvelopeDetector.h"

namespace rato
{

class PeakDetector final : public EnvelopeDetector
{
public:
    float processSample(float inputAbsolute) override
    {
        if (inputAbsolute > envelope)
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * inputAbsolute;
        else
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * inputAbsolute;

        return envelope;
    }
};

} // namespace rato

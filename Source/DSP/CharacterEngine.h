#pragma once

#include <cmath>
#include <algorithm>

namespace rato
{

enum class CharacterMode { Clean = 0, Punch, Rat };

class CharacterEngine
{
public:
    void prepare(double sampleRate);
    void reset();
    void setMode(CharacterMode newMode);

    float process(float inputLevel, float gainReductionDb);

private:
    CharacterMode mode = CharacterMode::Clean;

    // Shared signal statistics — always tracked, regardless of active mode.
    float peakEnv = 0.0f;
    float meanSquare = 0.0f;

    // Per-mode GR envelope state
    float grEnvelope = 0.0f;
    int compressingCounter = 0;
    float currentSustainFactor = 0.0f;

    // Shared detector coefficients (single set, all modes use same statistics)
    float peakDecay = 0.0f;
    float msCoeff = 0.0f;

    // Punch-specific coefficients
    float punchGrFastCoeff = 0.0f;
    float punchGrSlowCoeff = 0.0f;
    float invTransitionSamples = 0.0f;

    // Rat-specific coefficients
    float ratReleaseCoeff = 0.0f;

    void updateDetectorState(float inputLevel);
    float computeCrest() const;
    float processPunch(float grDb);
    float processRat(float grDb);

    static float decayCoeff(float timeMs, double sampleRate)
    {
        if (timeMs <= 0.0f || sampleRate <= 0.0)
            return 0.0f;
        return std::exp(-1000.0f / (timeMs * static_cast<float>(sampleRate)));
    }

    static float smoothCoeff(float timeMs, double sampleRate)
    {
        return 1.0f - decayCoeff(timeMs, sampleRate);
    }
};

} // namespace rato

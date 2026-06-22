#include "CharacterEngine.h"

namespace rato
{

void CharacterEngine::prepare(double sampleRate)
{
    // Shared detector — unified across all modes.
    // 2 ms peak follower catches transients without being noise-sensitive.
    // 20 ms RMS smoothing gives a stable energy reference.
    peakDecay = decayCoeff(2.0f, sampleRate);
    msCoeff   = smoothCoeff(20.0f, sampleRate);

    // Punch: fast/slow GR release blend, program-dependent.
    punchGrFastCoeff = decayCoeff(50.0f, sampleRate);
    punchGrSlowCoeff = decayCoeff(500.0f, sampleRate);

    const float transitionSamples = static_cast<float>(sampleRate) * 0.2f;
    invTransitionSamples = 1.0f / std::max(1.0f, transitionSamples);

    // Rat: single fast release, pumping behavior.
    ratReleaseCoeff = decayCoeff(15.0f, sampleRate);

    reset();
}

void CharacterEngine::reset()
{
    peakEnv = 0.0f;
    meanSquare = 0.0f;
    grEnvelope = 0.0f;
    compressingCounter = 0;
    currentSustainFactor = 0.0f;
}

void CharacterEngine::setMode(CharacterMode newMode)
{
    // No reset — internal state (peakEnv, meanSquare, grEnvelope) carries over
    // so mode changes behave as a character switch, not a compressor restart.
    mode = newMode;
}

void CharacterEngine::updateDetectorState(float inputLevel)
{
    peakEnv = std::max(inputLevel, peakEnv * peakDecay);
    meanSquare += (inputLevel * inputLevel - meanSquare) * msCoeff;
}

float CharacterEngine::computeCrest() const
{
    const float rmsLevel = std::sqrt(std::max(meanSquare, 0.0f));
    return peakEnv / std::max(rmsLevel, 1e-6f);
}

float CharacterEngine::process(float inputLevel, float gainReductionDb)
{
    // Detector state always alive — independent of active mode.
    updateDetectorState(inputLevel);

    switch (mode)
    {
        case CharacterMode::Clean:
            // Keep grEnvelope synced so Clean ↔ Punch/Rat transitions are seamless.
            grEnvelope = gainReductionDb;
            return gainReductionDb;

        case CharacterMode::Punch:
            return processPunch(gainReductionDb);

        case CharacterMode::Rat:
            return processRat(gainReductionDb);
    }
    return gainReductionDb;
}

// ---------------------------------------------------------------------------
//  Punch: transient enhancement + program-dependent release
// ---------------------------------------------------------------------------
float CharacterEngine::processPunch(float grDb)
{
    const float crest = computeCrest();

    // Transient enhancement: reduce GR on high-crest moments.
    float shaped = grDb;
    if (crest > 2.0f)
    {
        const float mod = std::max(1.0f - (crest - 2.0f) * 0.12f, 0.5f);
        shaped = grDb * mod;
    }

    // Program-dependent release.
    if (shaped <= grEnvelope)
    {
        grEnvelope = shaped;
        compressingCounter++;
    }
    else
    {
        if (compressingCounter > 0)
        {
            currentSustainFactor = std::min(
                static_cast<float>(compressingCounter) * invTransitionSamples, 1.0f);
            compressingCounter = 0;
        }

        const float adaptiveCoeff = punchGrFastCoeff
            + (punchGrSlowCoeff - punchGrFastCoeff) * currentSustainFactor;

        grEnvelope += (shaped - grEnvelope) * (1.0f - adaptiveCoeff);
    }

    return grEnvelope;
}

// ---------------------------------------------------------------------------
//  Rat: detector exaggeration + aggressive grab + pumping release
// ---------------------------------------------------------------------------
float CharacterEngine::processRat(float grDb)
{
    const float crest = computeCrest();

    // Detector exaggeration: 1.5x GR scaling.
    float shaped = std::max(grDb * 1.5f, -60.0f);

    // Nonlinear transient boost.
    if (crest > 1.5f)
    {
        const float mod = 1.0f + std::min((crest - 1.5f) * 0.2f, 0.8f);
        shaped = std::max(shaped * mod, -60.0f);
    }

    // Aggressive grab + pumping release.
    if (shaped <= grEnvelope)
        grEnvelope = shaped;
    else
        grEnvelope += (shaped - grEnvelope) * (1.0f - ratReleaseCoeff);

    return grEnvelope;
}

} // namespace rato

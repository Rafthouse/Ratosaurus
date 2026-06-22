#include "HarmonicsEngine.h"
#include <algorithm>
#include <cmath>

namespace rato
{

namespace
{
    // Per-model character profile. NO additional DSP — just a coefficient
    // re-weighting applied inside updateCoefficients().
    struct SaturationProfile
    {
        float driveMultiplier; // input gain scaling (extends or softens drive curve)
        float t2Boost;         // 2nd harmonic weight (even)
        float t3Boost;         // 3rd harmonic weight (odd)
        float t4Boost;         // 4th harmonic weight (even)
        float t5Boost;         // 5th harmonic weight (odd)
        float evenBias;        // shift toward even (+) or odd (-) regardless of Color
    };

    //                                drive   T2    T3    T4    T5   evenBias
    //                                ─────   ───   ───   ───   ───  ────────
    constexpr SaturationProfile kProfiles[] = {
        /* Off  */ { 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f },
        /* Tape */ { 0.8f,  1.5f, 0.7f, 0.5f, 0.3f, +0.3f },
        /* Tube */ { 1.0f,  0.8f, 1.5f, 0.6f, 0.4f, +0.1f },
        /* FET  */ { 1.2f,  1.0f, 1.0f, 1.3f, 1.5f,  0.0f },
        /* Rat  */ { 1.5f,  1.2f, 1.5f, 1.0f, 1.0f,  0.0f }
    };

    constexpr size_t kNumProfiles = sizeof(kProfiles) / sizeof(kProfiles[0]);
}

void HarmonicsEngine::prepare(const juce::dsp::ProcessSpec&)
{
    updateCoefficients();
}

void HarmonicsEngine::reset()
{
    // Stateless: no per-sample memory to clear.
}

void HarmonicsEngine::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (drive == 0.0f)
        return; // bit-identical bypass (modulo oversampling AA filter ripple)

    auto& block = context.getOutputBlock();
    const auto numChannels = block.getNumChannels();
    const auto numSamples  = block.getNumSamples();

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = block.getChannelPointer(ch);
        for (size_t i = 0; i < numSamples; ++i)
            data[i] = shape(data[i]);
    }
}

void HarmonicsEngine::setDrive(float newDrive)
{
    const float clamped = juce::jlimit(0.0f, 1.0f, newDrive);
    if (clamped != drive)
    {
        drive = clamped;
        updateCoefficients();
    }
}

void HarmonicsEngine::setDensity(float newDensity)
{
    const float clamped = juce::jlimit(0.0f, 1.0f, newDensity);
    if (clamped != density)
    {
        density = clamped;
        updateCoefficients();
    }
}

void HarmonicsEngine::setColor(float newColor)
{
    const float clamped = juce::jlimit(-1.0f, 1.0f, newColor);
    if (clamped != color)
    {
        color = clamped;
        updateCoefficients();
    }
}

void HarmonicsEngine::setSaturationModel(SaturationModel newModel)
{
    if (newModel != model)
    {
        model = newModel;
        updateCoefficients();
    }
}

void HarmonicsEngine::updateCoefficients()
{
    const auto profileIndex = static_cast<size_t>(model);
    const auto& profile = kProfiles[profileIndex < kNumProfiles ? profileIndex : 0];

    // Drive sweeps input gain. Model's driveMultiplier extends or softens
    // the drive curve (Tape softer, Rat hotter).
    driveGain = 1.0f + drive * 3.0f * profile.driveMultiplier;

    // Color: even-vs-odd balance, shifted by model's evenBias.
    // Clamped so neither amount goes negative; sum stays ≤ 1.
    const float evenAmount = juce::jlimit(0.0f, 1.0f,
        0.5f * (1.0f + color) + profile.evenBias);
    const float oddAmount  = juce::jlimit(0.0f, 1.0f,
        0.5f * (1.0f - color) - profile.evenBias);

    // Density: low-order (T2/T3) vs high-order (T4/T5) weighting.
    const float lowWeight  = (1.0f - density) * 0.5f;
    const float highWeight = density * 0.5f;

    // Drive scales total harmonic content; at Drive = 0 all higher Tn vanish.
    const float strength = drive;

    c1 = 1.0f;
    c2 = strength * evenAmount * lowWeight  * profile.t2Boost;
    c3 = strength * oddAmount  * lowWeight  * profile.t3Boost;
    c4 = strength * evenAmount * highWeight * profile.t4Boost;
    c5 = strength * oddAmount  * highWeight * profile.t5Boost;

    // Output compensation: keep peak output near unity by dividing by sum of
    // coefficient magnitudes. For a clamped input |x| ≤ 1, |T_n(x)| ≤ 1.
    const float total = std::abs(c1) + std::abs(c2) + std::abs(c3)
                      + std::abs(c4) + std::abs(c5);
    outputCompensation = 1.0f / std::max(total, 1.0f);
}

float HarmonicsEngine::shape(float x) const noexcept
{
    // Drive into the Chebyshev domain. Hard clamp keeps |x| ≤ 1 — values
    // outside this range cause T_n to diverge rapidly. Hard-clamp aliasing
    // is handled by the 4x oversampling around this stage.
    float xd = juce::jlimit(-1.0f, 1.0f, x * driveGain);

    const float x2 = xd * xd;
    const float x3 = xd * x2;
    const float x4 = x2 * x2;
    const float x5 = xd * x4;

    const float T1 = xd;
    const float T2 = 2.0f  * x2 - 1.0f;
    const float T3 = 4.0f  * x3 - 3.0f  * xd;
    const float T4 = 8.0f  * x4 - 8.0f  * x2 + 1.0f;
    const float T5 = 16.0f * x5 - 20.0f * x3 + 5.0f * xd;

    const float y = c1 * T1 + c2 * T2 + c3 * T3 + c4 * T4 + c5 * T5;
    return y * outputCompensation;
}

} // namespace rato

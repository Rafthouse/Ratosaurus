#pragma once

#include <juce_dsp/juce_dsp.h>

namespace rato
{

enum class SaturationModel { Off = 0, Tape, Tube, FET, Rat };

/**
    Chebyshev T1–T5 waveshaper. Drive scales input gain into the [-1, +1]
    nonlinear domain; Density splits energy between low-order (T2/T3) and
    high-order (T4/T5) harmonics; Color blends even (T2/T4) vs odd (T3/T5).

    SaturationModel applies a character profile that re-weights individual
    harmonics and shifts the drive curve. The model never duplicates DSP —
    it just adjusts how (Drive, Density, Color) map to c1..c5 coefficients.

    Stateless: no envelopes, no smoothing. At Drive = 0 the process call
    returns immediately, leaving the buffer untouched.

    Operates in the oversampled domain (block-based, ProcessContextReplacing).
*/
class HarmonicsEngine
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void process(const juce::dsp::ProcessContextReplacing<float>& context);

    void setDrive(float newDrive);
    void setDensity(float newDensity);
    void setColor(float newColor);
    void setSaturationModel(SaturationModel newModel);

private:
    float drive   = 0.0f;
    float density = 0.5f;
    float color   = 0.0f;
    SaturationModel model = SaturationModel::Off;

    // Cached coefficients, recomputed whenever Drive / Density / Color changes.
    float driveGain = 1.0f;
    float c1 = 1.0f;
    float c2 = 0.0f;
    float c3 = 0.0f;
    float c4 = 0.0f;
    float c5 = 0.0f;
    float outputCompensation = 1.0f;

    void updateCoefficients();
    float shape(float x) const noexcept;
};

} // namespace rato

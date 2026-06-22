#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace rato
{

/**
    Stage 10 replacement for LevelMeterComponent.

    Two modes:
        Mode::Level         — bottom-up fill, amber gradient
        Mode::GainReduction — top-down fill, single-tone amber

    No DSP — fed dB values from outside (e.g., editor's Timer). Identical
    interface to LevelMeterComponent so migration in 10D is a type swap.

    No allocations on paint. No state shared with audio thread.
*/
class RatoMeter final : public juce::Component
{
public:
    enum class Mode { Level, GainReduction };

    RatoMeter();
    ~RatoMeter() override = default;

    void setMode(Mode newMode);
    void setLevelDb(float dB);

    void paint(juce::Graphics&) override;

private:
    Mode  mode       = Mode::Level;
    float currentDb  = -100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatoMeter)
};

} // namespace rato

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <memory>

namespace rato
{

/**
    Saturation model selector — 5 illuminated hardware-style tiles
    (OFF / TAPE / TUBE / FET / RAT) per Ratosaurus Plugin v4.dc.html.

    Each tile: LED dot + line-art icon + label, inside a bevelled glass
    panel. Active tile lights up (RAT = hot magenta, others = purple/cream).
    Click a tile to set the "saturationModel" parameter.

    Icons come from AssetCache (handoff SVGs, stroke baked to #000000 and
    re-tinted per state via Drawable::replaceColour).
*/
class RatoSatTiles final : public juce::Component
{
public:
    explicit RatoSatTiles(juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& paramId);

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    static constexpr int kNumTiles = 5;

    juce::RangedAudioParameter* parameter = nullptr;
    juce::ParameterAttachment attachment;
    int currentIndex = 0;

    std::array<std::unique_ptr<juce::Drawable>, kNumTiles> icons;
    std::array<juce::String, kNumTiles> labels { "OFF", "TAPE", "TUBE", "FET", "RAT" };

    void onParameterChanged(float denormalised);
    int tileAtX(int x) const;
    juce::Rectangle<float> tileBounds(int i) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatoSatTiles)
};

} // namespace rato

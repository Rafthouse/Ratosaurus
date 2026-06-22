#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace rato
{

/**
    Thin juce::ComboBox subclass for use with RatosaurusLookAndFeel.

    Currently has no per-instance customisation — the visual identity is
    fully driven by LookAndFeel::drawComboBox / positionComboBoxText.

    Stage 10D will use this in place of juce::ComboBox in PluginEditor so
    future styling has a single place to evolve.
*/
class RatoComboBox final : public juce::ComboBox
{
public:
    RatoComboBox() = default;
    ~RatoComboBox() override = default;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatoComboBox)
};

} // namespace rato

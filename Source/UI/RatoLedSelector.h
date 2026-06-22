#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace rato
{

/**
    LED-style radio selector per Ratosaurus Plugin v4.dc.html.

    Vertical list of options. Each row = LED dot + label.
    Active row: hot-magenta LED with glow + cream label.
    Inactive rows: dim grey LED + muted text.
    Click a row to set the parameter.

    Bound to APVTS via juce::ParameterAttachment.
*/
class RatoLedSelector final : public juce::Component
{
public:
    RatoLedSelector(juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& paramId,
                    const juce::String& titleText,
                    const juce::StringArray& optionLabels);

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    juce::String title;
    juce::StringArray options;

    juce::RangedAudioParameter* parameter = nullptr;
    juce::ParameterAttachment attachment;
    int currentIndex = 0;

    void onParameterChanged(float normalised);

    // Layout is derived from the current component bounds so the selector,
    // its LEDs and its typography scale as one unit with the editor.
    int rowAtY(int y) const;

    float referenceHeight() const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatoLedSelector)
};

} // namespace rato

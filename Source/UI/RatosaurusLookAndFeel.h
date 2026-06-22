#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "RatosaurusTheme.h"
#include <memory>

namespace rato
{

/**
    LookAndFeel for the Ratosaurus visual identity.

    Overrides only the JUCE LAF methods needed to recolour the existing widget
    set. Does NOT replace widgets — RatoKnob / RatoComboBox / RatoMeter are
    independent components that may use this LAF (Stage 10D will wire them up).

    This class is allocated as a member of the editor and assigned via
    setLookAndFeel(&...). The destructor relies on the editor calling
    setLookAndFeel(nullptr) before destruction.

    No state, no allocations after construction.
*/
class RatosaurusLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    RatosaurusLookAndFeel();
    ~RatosaurusLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics&,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider&) override;

    void drawComboBox(juce::Graphics&,
                      int width, int height,
                      bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;

    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;

    void drawLabel(juce::Graphics&, juce::Label&) override;

    void drawButtonBackground(juce::Graphics&,
                              juce::Button&,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics&,
                        juce::TextButton&,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;

private:
    std::unique_ptr<juce::Drawable> knobBase;
    std::unique_ptr<juce::Drawable> knobRotor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatosaurusLookAndFeel)
};

} // namespace rato

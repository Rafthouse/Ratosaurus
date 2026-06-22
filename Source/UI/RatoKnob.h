#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace rato
{

/**
    Visual style for the knob, matching the five concepts in
    Ratosaurus Knobs.dc.html. The style is stored in the Slider's
    property bag so the RatosaurusLookAndFeel can pick the paint
    routine without a dynamic_cast.
*/
enum class KnobStyle
{
    BakelitePointer = 0,  // #1 — fat matte bakelite + brass cap (LA-2A / Fairchild)
    BrushedAlu      = 1,  // #2 — brushed aluminium skirt (UREI 1176)
    SovietFluted    = 2,  // #3 — olive-black phenolic with vertical flutes (default)
    OxbloodCap      = 3,  // #4 — glossy oxblood + brass dome cap (Pultec)
    LabDial         = 4   // #5 — military instrument dial, knurled collar
};

inline constexpr const char* knobStyleProperty = "ratoKnobStyle";

/**
    juce::Slider subclass pre-configured for Ratosaurus.
    Rotary, 270° sweep, text box below. Default style = SovietFluted.
*/
class RatoKnob final : public juce::Slider
{
public:
    explicit RatoKnob(KnobStyle initialStyle = KnobStyle::SovietFluted)
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                            juce::MathConstants<float>::pi * 2.75f,
                            true);
        setStyle(initialStyle);
    }

    ~RatoKnob() override = default;

    void setStyle(KnobStyle s)
    {
        style = s;
        getProperties().set(knobStyleProperty, static_cast<int>(s));
        repaint();
    }

    KnobStyle getStyle() const noexcept { return style; }

private:
    KnobStyle style = KnobStyle::SovietFluted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatoKnob)
};

} // namespace rato

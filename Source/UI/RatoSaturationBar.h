#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include <array>
#include "RatosaurusTheme.h"

namespace rato
{

// ============================================================================
/** 5-button saturation model selector — lamp-style with LED+icon per model.

    Behaves as a radio group: exactly one button active at a time.
    Connects to APVTS via a ValueAttachment or direct parameter write.

    Each button shows:
      - An SVG icon (from AssetCache) or Unicode fallback glyph
      - A label (Off, Tape, Tube, FET, Rat)
      - A magenta glow LED when active
      - A purple hover ring
*/
class RatoSaturationBar final : public juce::Component
{
public:
    static constexpr int kNumModels = 5;

    static constexpr const char* kModelNames[kNumModels] =
    {
        "Off", "Tape", "Tube", "FET", "Rat"
    };

    static constexpr const char* kModelGlyphs[kNumModels] =
    {
        "O",  // Off (circle-like)
        "Tp", // Tape (reel)
        "Tb", // Tube (valve)
        "F",  // FET
        "R"   // Rat (star)
    };

    // ====================================================================
    enum class ColourIds
    {
        backgroundOff     = 0x2A10001,
        backgroundActive  = 0x2A10002,
        borderOff         = 0x2A10003,
        borderActive      = 0x2A10004,
        textOff           = 0x2A10005,
        textActive        = 0x2A10006,
        iconOff           = 0x2A10007,
        iconActive        = 0x2A10008,
        ledOff            = 0x2A10009,
        ledActive         = 0x2A1000A
    };

    // ====================================================================
    RatoSaturationBar()
    {
        for (int i = 0; i < kNumModels; ++i)
        {
            buttons[i].setName(juce::String(kModelNames[i]));
            buttons[i].setIndex(i);
            buttons[i].onClick = [this, i]() { selectModel(i); };
            addAndMakeVisible(buttons[i]);
        }

        selectModel(0);

        // Default colour scheme from Design System palette
        setColour(static_cast<int>(ColourIds::backgroundOff),    theme::col(theme::colour::stationDim));
        setColour(static_cast<int>(ColourIds::backgroundActive), theme::col(theme::colour::panelMid));
        setColour(static_cast<int>(ColourIds::borderOff),        theme::col(theme::colour::brassDark));
        setColour(static_cast<int>(ColourIds::borderActive),     theme::col(theme::colour::stationFrame));
        setColour(static_cast<int>(ColourIds::textOff),          theme::col(theme::colour::brassDark));
        setColour(static_cast<int>(ColourIds::textActive),       theme::col(theme::colour::stationLit));
        setColour(static_cast<int>(ColourIds::iconOff),          theme::col(theme::colour::brassPatina));
        setColour(static_cast<int>(ColourIds::iconActive),       theme::col(theme::colour::stationLit));
        setColour(static_cast<int>(ColourIds::ledOff),           theme::col(theme::colour::brassDark));
        setColour(static_cast<int>(ColourIds::ledActive),        theme::col(theme::colour::stationLit));
    }

    ~RatoSaturationBar() override = default;

    // ====================================================================
    int  getSelectedIndex() const noexcept { return selectedIndex; }
    void setSelectedIndex(int idx, juce::NotificationType n = juce::sendNotification)
    {
        selectModel(idx);
        if (n == juce::sendNotification && onModelChange != nullptr)
            onModelChange(selectedIndex);
    }

    std::function<void(int)> onModelChange;

    // ====================================================================
    void paint(juce::Graphics&) override {}
    void resized() override
    {
        const auto area = getLocalBounds();
        const int btnW = area.getWidth() / kNumModels;
        for (int i = 0; i < kNumModels; ++i)
            buttons[i].setBounds(i * btnW, 0, btnW, area.getHeight());
    }

private:
    // ====================================================================
    class ModelButton final : public juce::Button
    {
    public:
        ModelButton() : Button("") {}

        void setIndex(int i) noexcept { index = i; }
        int  getIndex() const noexcept { return index; }

        void paintButton(juce::Graphics& g, bool isHovering, bool isDown) override
        {
            const auto bounds = getLocalBounds().toFloat().reduced(1.0f);
            const bool active = getToggleState();

            // Background — dark glass tile; active gets purple glow
            auto bgCol = findColour(static_cast<int>(ColourIds::backgroundOff));
            if (active)
            {
                bgCol = findColour(static_cast<int>(ColourIds::backgroundActive));
                // Inner glow when active
                auto glowBounds = bounds.reduced(2.0f);
                g.setColour(theme::col(theme::colour::amberMid).withAlpha(0.12f));
                g.fillRoundedRectangle(glowBounds, 3.0f);
            }
            g.setColour(bgCol);
            g.fillRoundedRectangle(bounds, 3.0f);

            // Border — bronze normally, purple bright when active
            auto borderCol = findColour(static_cast<int>(ColourIds::borderOff));
            if (active || isHovering)
                borderCol = findColour(static_cast<int>(ColourIds::borderActive));
            g.setColour(borderCol);
            g.drawRoundedRectangle(bounds, 3.0f, active ? 1.2f : 0.6f);

            // LED dot at top
            const auto ledCol = active
                ? findColour(static_cast<int>(ColourIds::ledActive))
                : findColour(static_cast<int>(ColourIds::ledOff));
            const float ledR = 3.0f;
            const float ledX = bounds.getCentreX();
            const float ledY = bounds.getY() + 8.0f;
            g.setColour(ledCol);
            g.fillEllipse(ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);
            if (active)
            {
                g.setColour(ledCol.withAlpha(0.3f));
                g.fillEllipse(ledX - ledR * 2.0f, ledY - ledR * 2.0f,
                              ledR * 4.0f, ledR * 4.0f);
            }

            // Icon glyph (text-based for now; SVG icons will replace later)
            const int idx = std::clamp(index, 0, kNumModels - 1);
            auto iconCol = findColour(static_cast<int>(ColourIds::iconOff));
            if (active) iconCol = findColour(static_cast<int>(ColourIds::iconActive));

            g.setColour(iconCol);
            auto font = juce::Font(juce::FontOptions(bounds.getHeight() * 0.25f));
            g.setFont(font);
            g.drawText(juce::String(kModelGlyphs[idx]),
                       bounds.withTrimmedTop(bounds.getHeight() * 0.22f).reduced(2, 0),
                       juce::Justification::centred, false);

            // Model name at bottom
            auto nameCol = findColour(static_cast<int>(ColourIds::textOff));
            if (active) nameCol = findColour(static_cast<int>(ColourIds::textActive));
            g.setColour(nameCol);
            auto nameFont = juce::Font(juce::FontOptions(theme::font::engravedLabel));
            g.setFont(nameFont);
            g.drawText(juce::String(kModelNames[idx]),
                       bounds.withTrimmedBottom(6.0f).reduced(2, 0),
                       juce::Justification::centred, false);
        }

    private:
        int index = 0;
    };

    // ====================================================================
    void selectModel(int idx)
    {
        if (idx < 0 || idx >= kNumModels)
            return;

        for (int i = 0; i < kNumModels; ++i)
            buttons[i].setToggleState(i == idx, juce::dontSendNotification);

        selectedIndex = idx;
    }

    std::array<ModelButton, kNumModels> buttons;
    int selectedIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatoSaturationBar)
};

} // namespace rato

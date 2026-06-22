#include "RatoLedSelector.h"
#include "RatoFonts.h"

namespace rato
{

namespace
{
    const juce::Colour kPanel        { 0xFF1F1428 };
    const juce::Colour kBorder       { 0xFF7C4F96 };
    const juce::Colour kTitleColour  { 0xFFC9B8A0 };
    const juce::Colour kActiveLed    { 0xFFFF8DF5 };  // hot magenta
    const juce::Colour kInactiveLed  { 0xFF2A1830 };  // dark
    const juce::Colour kActiveText   { 0xFFE8DCC2 };  // cream
    const juce::Colour kInactiveText { 0xFF8C7A96 };  // dim purple-grey
    const juce::Colour kGlowMagenta  { 0xFFFF8DF5 };
}

RatoLedSelector::RatoLedSelector(juce::AudioProcessorValueTreeState& apvts,
                                 const juce::String& paramId,
                                 const juce::String& titleText,
                                 const juce::StringArray& optionLabels)
    : title(titleText),
      options(optionLabels),
      parameter(apvts.getParameter(paramId)),
      attachment(*parameter,
                 [this](float v) { onParameterChanged(v); })
{
    jassert(parameter != nullptr);
    attachment.sendInitialUpdate();
}

void RatoLedSelector::onParameterChanged(float denormalisedValue)
{
    // ParameterAttachment already converts 0..1 -> the parameter's natural
    // range, so for a choice parameter this is the index as a float.
    const int idx = juce::jlimit(0, options.size() - 1,
                                 static_cast<int>(std::round(denormalisedValue)));
    if (idx != currentIndex)
    {
        currentIndex = idx;
        repaint();
    }
}

void RatoLedSelector::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float scale = bounds.getHeight() / referenceHeight();
    const float titleH = 16.0f * scale;
    const float titleGap = 6.0f * scale;
    const float rowStartY = titleH + titleGap;
    const float rowH = (bounds.getHeight() - rowStartY)
                     / static_cast<float>(juce::jmax(1, options.size()));
    const float ledSize = 7.0f * scale;

    // Title at top
    g.setColour(kTitleColour);
    g.setFont(Fonts::oswald(13.5f * scale, true));
    g.drawText(title,
               bounds.withHeight(titleH),
               juce::Justification::topLeft, false);

    // Rows
    for (int i = 0; i < options.size(); ++i)
    {
        const float y = rowStartY + static_cast<float>(i) * rowH;
        const auto rowR = juce::Rectangle<float>(0.0f, y, bounds.getWidth(), rowH);

        const bool active = (i == currentIndex);

        // LED dot (left)
        const float ledCx = ledSize * 0.5f + 6.0f * scale;
        const float ledCy = y + rowH * 0.5f;
        const float ledR = ledSize * 0.5f;

        if (active)
        {
            // Glow halo (multi-pass alpha for faux-blur)
            for (int g_i = 4; g_i >= 0; --g_i)
            {
                g.setColour(kGlowMagenta.withAlpha(0.18f - g_i * 0.03f));
                const float er = ledR + (2.0f + static_cast<float>(g_i)) * scale;
                g.fillEllipse(ledCx - er, ledCy - er, er * 2.0f, er * 2.0f);
            }
            g.setColour(kActiveLed);
        }
        else
        {
            g.setColour(kInactiveLed);
        }
        g.fillEllipse(ledCx - ledR, ledCy - ledR, ledR * 2.0f, ledR * 2.0f);

        // Label text (right of LED)
        g.setColour(active ? kActiveText : kInactiveText);
        g.setFont(Fonts::oswald(14.0f * scale, active));
        const auto textR = rowR.withTrimmedLeft(ledR * 2.0f + 12.0f * scale);
        g.drawText(options[i], textR.toNearestInt(),
                   juce::Justification::centredLeft, false);
    }
}

int RatoLedSelector::rowAtY(int y) const
{
    const float scale = static_cast<float>(getHeight()) / referenceHeight();
    const float rowStartY = (16.0f + 6.0f) * scale;
    if (static_cast<float>(y) < rowStartY) return -1;

    const float rowH = (static_cast<float>(getHeight()) - rowStartY)
                     / static_cast<float>(juce::jmax(1, options.size()));
    if (rowH <= 0.0f) return -1;

    const int idx = static_cast<int>((static_cast<float>(y) - rowStartY) / rowH);
    if (idx < 0 || idx >= options.size()) return -1;
    return idx;
}

float RatoLedSelector::referenceHeight() const noexcept
{
    // The v4 design uses 82 px for three rows and 62 px for two rows.
    return options.size() >= 3 ? 82.0f : 62.0f;
}

void RatoLedSelector::mouseDown(const juce::MouseEvent& e)
{
    const int idx = rowAtY(e.y);
    if (idx >= 0 && idx != currentIndex && parameter != nullptr)
    {
        attachment.setValueAsCompleteGesture(static_cast<float>(idx));
    }
}

} // namespace rato

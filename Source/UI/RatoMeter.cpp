#include "RatoMeter.h"
#include "RatosaurusTheme.h"

namespace rato
{

namespace c = ::rato::theme::colour;

RatoMeter::RatoMeter() = default;

void RatoMeter::setMode(Mode newMode)
{
    mode = newMode;
    repaint();
}

void RatoMeter::setLevelDb(float dB)
{
    if (dB != currentDb)
    {
        currentDb = dB;
        repaint();
    }
}

void RatoMeter::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // --- Meter well (recessed dark window) ---
    g.setColour(theme::col(c::meterWell));
    g.fillRoundedRectangle(bounds, 1.5f);
    g.setColour(theme::col(c::wellBlack).withAlpha(0.6f));
    g.drawRoundedRectangle(bounds, 1.5f, 1.0f);

    const auto fillArea = bounds.reduced(2.0f);
    const float segGap  = 1.5f;           // visible gap between segments
    constexpr int kSegments = 20;

    if (mode == Mode::Level)
    {
        constexpr float minDb = -60.0f;
        constexpr float maxDb =   6.0f;
        const float clamped    = juce::jlimit(minDb, maxDb, currentDb);
        const float normalised = (clamped - minDb) / (maxDb - minDb);

        const float segH = (fillArea.getHeight() - segGap * (kSegments - 1))
                         / static_cast<float>(kSegments);
        const int litSegs = juce::jlimit(0, kSegments,
            static_cast<int>(std::ceil(normalised * kSegments)));

        for (int i = 0; i < kSegments; ++i)
        {
            const float y = fillArea.getBottom()
                          - (segH + segGap) * (i + 1);

            if (i < litSegs)
            {
                // Gradient: bottom = levelLow (purple), top = levelPeak (magenta)
                juce::Colour segCol;
                const float t = static_cast<float>(i) / static_cast<float>(kSegments);
                if (t < 0.5f)
                    segCol = theme::col(c::levelLow).interpolatedWith(theme::col(c::levelMid), t * 2.0f);
                else if (t < 0.75f)
                    segCol = theme::col(c::levelMid).interpolatedWith(theme::col(c::levelHigh), (t - 0.5f) * 4.0f);
                else
                    segCol = theme::col(c::levelHigh).interpolatedWith(theme::col(c::levelPeak), (t - 0.75f) * 4.0f);

                g.setColour(segCol);

                // Glow on top-lit segments
                if (i >= kSegments - 2)
                    g.setColour(segCol.brighter(0.3f));
            }
            else
            {
                // Dim unlit segments
                g.setColour(theme::col(c::meterWell).brighter(0.08f));
            }

            g.fillRect(fillArea.getX(), y, fillArea.getWidth(), segH);

            // Sub-glow on lit segments (subtle)
            if (i < litSegs && i >= kSegments - 3)
            {
                g.setColour(theme::col(c::levelPeak).withAlpha(0.15f));
                g.fillRect(fillArea.getX(), y, fillArea.getWidth(), segH);
            }
        }
    }
    else // GainReduction — top-down, horizontal bar preferable but kept vertical for existing layout
    {
        constexpr float maxReduction = 24.0f;
        const float reduction  = juce::jmax(0.0f, -currentDb);
        const float normalised = juce::jmin(reduction / maxReduction, 1.0f);

        constexpr int grSegments = 30;
        const float segW = (fillArea.getWidth() - segGap * (grSegments - 1))
                         / static_cast<float>(grSegments);
        const int litSegs = juce::jlimit(0, grSegments,
            static_cast<int>(std::ceil(normalised * grSegments)));

        for (int i = 0; i < grSegments; ++i)
        {
            const float x = fillArea.getRight() - (segW + segGap) * (i + 1);

            if (i < litSegs)
            {
                g.setColour(theme::col(c::grFill));
                if (i >= grSegments - 4)
                    g.setColour(theme::col(c::grFill).brighter(0.3f));
            }
            else
            {
                g.setColour(theme::col(c::meterWell).brighter(0.06f));
            }

            g.fillRect(x, fillArea.getY(), segW, fillArea.getHeight());
        }
    }
}

} // namespace rato

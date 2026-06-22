#include "CalibratedVUMeter.h"
#include "RatoFonts.h"
#include <array>
#include <cmath>

namespace rato
{

namespace
{
constexpr float startAngle = -2.35619449f; // -135 degrees
constexpr float endAngle   = -0.78539816f; //  -45 degrees

juce::Point<float> pointOnArc(juce::Point<float> pivot, float radius, float angle)
{
    return pivot.translated(std::cos(angle) * radius, std::sin(angle) * radius);
}

void drawArcZone(juce::Graphics& g, juce::Point<float> pivot, float radius,
                 float fromNorm, float toNorm, juce::Colour colour, float thickness)
{
    juce::Path path;
    constexpr int steps = 32;
    for (int i = 0; i <= steps; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        const float norm = juce::jmap(t, fromNorm, toNorm);
        const auto p = pointOnArc(pivot, radius, juce::jmap(norm, startAngle, endAngle));
        if (i == 0) path.startNewSubPath(p); else path.lineTo(p);
    }
    g.setColour(colour);
    g.strokePath(path, juce::PathStrokeType(thickness,
                                            juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded));
}
}

CalibratedVUMeter::CalibratedVUMeter(VUMeterBallistics& source,
                                     juce::AudioProcessorValueTreeState& apvts)
    : meterSource(source), state(apvts)
{
    calibrationBox.addItem("-18 dBFS = 0 VU", 1);
    calibrationBox.addItem("-24 dBFS = 0 VU", 2);
    calibrationBox.addItem("-20 dBFS = 0 VU", 3);
    calibrationBox.onChange = [this] { repaint(); };
    // Calibration remains an automatable APVTS parameter, but the technical
    // ComboBox is intentionally not exposed on the v4 faceplate. It used to
    // consume the bottom of the meter and visually read as a second widget.
    calibrationBox.setVisible(false);

    calibrationAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            state, "vuCalibration", calibrationBox);
}

void CalibratedVUMeter::processBlock(const juce::AudioBuffer<float>& buffer) noexcept
{
    meterSource.processBlock(buffer, getCalibrationOffset());
}

void CalibratedVUMeter::updateDisplay()
{
    const float next = meterSource.getCurrentVu();
    if (std::abs(next - displayedVu) > 0.01f)
    {
        displayedVu = next;
        repaint();
    }
}

float CalibratedVUMeter::getCalibrationOffset() const noexcept
{
    const auto* value = state.getRawParameterValue("vuCalibration");
    const int choice = value != nullptr ? static_cast<int>(value->load()) : 0;
    return VUMeterBallistics::calibrationOffsetForChoice(choice);
}

float CalibratedVUMeter::vuToNormalised(float vu) noexcept
{
    return juce::jlimit(0.0f, 1.0f, (vu + 20.0f) / 23.0f);
}

float CalibratedVUMeter::normalisedToAngle(float normalised) noexcept
{
    return juce::jmap(normalised, startAngle, endAngle);
}

void CalibratedVUMeter::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float corner = juce::jmax(3.0f, bounds.getHeight() * 0.035f);

    g.setColour(juce::Colour(0xFF1E1E1E));
    g.fillRoundedRectangle(bounds, corner);
    g.setColour(juce::Colour(0xFF704A80));
    g.drawRoundedRectangle(bounds.reduced(0.5f), corner, 1.0f);

    const float uiScale = bounds.getHeight() / 164.0f;
    calibrationHitArea = juce::Rectangle<float>(
        bounds.getX() + 7.0f * uiScale,
        bounds.getY() + 7.0f * uiScale,
        48.0f * uiScale,
        17.0f * uiScale);

    const int calibrationChoice = calibrationBox.getSelectedId();
    const juce::String calibrationText = calibrationChoice == 2 ? "CAL -24"
                                         : calibrationChoice == 3 ? "CAL -20"
                                                                  : "CAL -18";
    g.setColour(juce::Colour(0xFF0C0710).withAlpha(0.92f));
    g.fillRoundedRectangle(calibrationHitArea, 3.0f * uiScale);
    g.setColour(juce::Colour(0xFF704A80));
    g.drawRoundedRectangle(calibrationHitArea, 3.0f * uiScale,
                           juce::jmax(1.0f, uiScale));
    g.setColour(juce::Colour(0xFFD8C9A8));
    g.setFont(Fonts::mono(juce::jmax(7.0f, 8.5f * uiScale)));
    g.drawText(calibrationText, calibrationHitArea.toNearestInt(),
               juce::Justification::centred, false);

    const auto meterArea = bounds.reduced(5.0f, 4.0f);
    const auto pivot = juce::Point<float>(meterArea.getCentreX(), meterArea.getBottom() - 7.0f);
    const float radius = juce::jmin(meterArea.getWidth() * 0.43f,
                                    meterArea.getHeight() * 0.78f);
    const float thickness = juce::jmax(2.0f, radius * 0.045f);

    // -20..-3 green, -3..0 yellow, 0..+3 red.
    drawArcZone(g, pivot, radius, vuToNormalised(-20.0f), vuToNormalised(-3.0f),
                juce::Colour(0xFF55B878), thickness);
    drawArcZone(g, pivot, radius, vuToNormalised(-3.0f), vuToNormalised(0.0f),
                juce::Colour(0xFFE0B84D), thickness);
    drawArcZone(g, pivot, radius, vuToNormalised(0.0f), vuToNormalised(3.0f),
                juce::Colour(0xFFE05A5A), thickness);

    static constexpr std::array<float, 9> tickValues {
        -20.0f, -10.0f, -7.0f, -5.0f, -3.0f, 0.0f, 1.0f, 2.0f, 3.0f
    };

    g.setFont(Fonts::mono(juce::jmax(7.0f, radius * 0.09f)));
    for (const float value : tickValues)
    {
        const float angle = normalisedToAngle(vuToNormalised(value));
        const auto outer = pointOnArc(pivot, radius - thickness * 0.6f, angle);
        const auto inner = pointOnArc(pivot, radius - thickness * 2.4f, angle);
        const auto label = pointOnArc(pivot, radius - thickness * 4.7f, angle);

        g.setColour(value > 0.0f ? juce::Colour(0xFFE05A5A)
                                : juce::Colour(0xFFD8C9A8));
        g.drawLine({ inner, outer }, juce::jmax(1.0f, radius * 0.012f));

        const juce::String text = value > 0.0f ? "+" + juce::String(static_cast<int>(value))
                                                : juce::String(static_cast<int>(value));
        const float labelW = juce::jmax(20.0f, radius * 0.30f);
        const float labelH = juce::jmax(12.0f, radius * 0.16f);
        g.drawText(text,
                   juce::Rectangle<float>(label.x - labelW * 0.5f,
                                          label.y - labelH * 0.5f,
                                          labelW, labelH).toNearestInt(),
                   juce::Justification::centred, false);
    }

    g.setColour(juce::Colour(0xFFD8C9A8));
    g.setFont(Fonts::oswald(juce::jmax(11.0f, radius * 0.16f), true));
    g.drawText("VU", meterArea.withTrimmedTop(meterArea.getHeight() * 0.48f).toNearestInt(),
               juce::Justification::centredTop, false);

    const float needleAngle = normalisedToAngle(vuToNormalised(displayedVu));
    const auto needleEnd = pointOnArc(pivot, radius * 0.88f, needleAngle);
    g.setColour(juce::Colours::black.withAlpha(0.65f));
    g.drawLine({ pivot.translated(1.5f, 2.0f), needleEnd.translated(1.5f, 2.0f) },
               juce::jmax(2.0f, radius * 0.025f));
    g.setColour(juce::Colours::white);
    g.drawLine({ pivot, needleEnd }, juce::jmax(1.4f, radius * 0.018f));
    g.fillEllipse(pivot.x - 4.0f, pivot.y - 4.0f, 8.0f, 8.0f);

    auto readoutArea = meterArea;
    const auto readout = readoutArea.removeFromTop(22.0f).removeFromRight(58.0f);
    g.setColour(juce::Colour(0xFFD8C9A8));
    g.setFont(Fonts::mono(10.0f));
    g.drawText(juce::String(displayedVu, 1) + " VU", readout.toNearestInt(),
               juce::Justification::centredRight, false);
}

void CalibratedVUMeter::resized()
{
    calibrationBox.setBounds({});
}

void CalibratedVUMeter::mouseDown(const juce::MouseEvent& event)
{
    if (! calibrationHitArea.contains(event.position))
        return;

    const int current = juce::jlimit(1, 3, calibrationBox.getSelectedId());
    calibrationBox.setSelectedId(current % 3 + 1, juce::sendNotificationSync);
    repaint();
}

} // namespace rato

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/VUMeterBallistics.h"
#include "RatoComboBox.h"
#include <memory>

namespace rato
{

class CalibratedVUMeter final : public juce::Component
{
public:
    CalibratedVUMeter(VUMeterBallistics& source,
                      juce::AudioProcessorValueTreeState& apvts);
    ~CalibratedVUMeter() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;

    /** Optional forwarding API for non-plugin use. The plugin processor calls
        VUMeterBallistics directly so no Component is touched by the audio thread. */
    void processBlock(const juce::AudioBuffer<float>& buffer) noexcept;

    /** Called from PluginEditor::timerCallback at 30 Hz. */
    void updateDisplay();

private:
    VUMeterBallistics& meterSource;
    juce::AudioProcessorValueTreeState& state;
    RatoComboBox calibrationBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> calibrationAttachment;
    float displayedVu = -60.0f;
    juce::Rectangle<float> calibrationHitArea;

    float getCalibrationOffset() const noexcept;
    static float vuToNormalised(float vu) noexcept;
    static float normalisedToAngle(float normalised) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CalibratedVUMeter)
};

} // namespace rato

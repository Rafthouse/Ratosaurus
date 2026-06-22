#pragma once

#include "PluginProcessor.h"
#include "UI/RatosaurusLookAndFeel.h"
#include "UI/RatoKnob.h"
#include "UI/RatoComboBox.h"
#include "UI/RatoMeter.h"
#include "UI/CalibratedVUMeter.h"
#include "UI/RatoLedSelector.h"
#include "UI/RatoSatTiles.h"
#include "UI/AssetCache.h"

class RatosaurusEditor final : public juce::AudioProcessorEditor,
                               private juce::Timer
{
public:
    explicit RatosaurusEditor(RatosaurusProcessor&);
    ~RatosaurusEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RatosaurusProcessor& processor;

    rato::RatosaurusLookAndFeel lookAndFeel;

    // --- Top bar (Stage 8 + 9 + 10E bypass) ---
    rato::RatoComboBox presetCombo;
    rato::RatoComboBox oversamplingCombo;
    juce::TextButton aButton, bButton, copyButton, bypassButton;
    juce::Label      presetLabel;

    void refreshPresetCombo();
    void refreshAbButtons();

    // --- I/O ---
    rato::RatoKnob  inputKnob, outputKnob;
    juce::Label     inputLabel, outputLabel;
    rato::RatoMeter inputMeterUI, outputMeterUI;
    rato::CalibratedVUMeter vuMeter;

    // --- Compressor ---
    rato::RatoKnob     thresholdKnob, ratioKnob, attackKnob, releaseKnob;
    juce::Label        thresholdLabel, ratioLabel, attackLabel, releaseLabel;
    std::unique_ptr<rato::RatoLedSelector> kneeSelector;
    std::unique_ptr<rato::RatoLedSelector> detectorSelector;
    rato::RatoMeter    grMeterUI;
    juce::Label        grLabel;

    // --- Character ---
    std::unique_ptr<rato::RatoLedSelector> characterSelector;

    // --- Harmonics + Saturation ---
    rato::RatoKnob     driveKnob, densityKnob, colorKnob;
    juce::Label        driveLabel, densityLabel, colorLabel;
    std::unique_ptr<rato::RatoSatTiles> satTiles;

    // --- Section headers ---
    juce::Label sectionIO, sectionComp, sectionChar, sectionSat;

    // --- Stage 10D: SVG assets via AssetCache ---
    std::unique_ptr<juce::Drawable> logoDrawable;
    std::unique_ptr<juce::Drawable> signatureDrawable;
    std::unique_ptr<juce::Drawable> currentSatIcon;
    int  lastSatModelIndex = -1;

    juce::Rectangle<int> logoBounds;
    juce::Rectangle<int> signatureBounds;
    juce::Rectangle<int> satIconBounds;

    // v4: section frame outlines for floating-border paint
    juce::Rectangle<int> sectionIOFrame;
    juce::Rectangle<int> sectionCompFrame;
    juce::Rectangle<int> sectionSatFrame;
    juce::Rectangle<int> bottomBarFrame;

    // --- v5: faceplate background image (the v4 render = static art master) ---
    std::unique_ptr<juce::Drawable> faceplateDrawable;

    // --- Stage 10E: animation clock for breathing pulse / hover anim ---
    double animationTimeSec = 0.0;

    void updateSaturationIcon();

    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment>   inputAtt, outputAtt;
    std::unique_ptr<SliderAttachment>   thresholdAtt, ratioAtt, attackAtt, releaseAtt;
    std::unique_ptr<SliderAttachment>   driveAtt, densityAtt, colorAtt;
    std::unique_ptr<ButtonAttachment>   bypassAtt;
    std::unique_ptr<ComboBoxAttachment> oversamplingAtt;

    void setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& text);
    void setupCombo(juce::ComboBox& combo, juce::Label& label,
                    const juce::String& text, const juce::StringArray& items);
    void setupSectionHeader(juce::Label& header, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatosaurusEditor)
};

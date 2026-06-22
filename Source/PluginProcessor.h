#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include "DSP/CompressorEngine.h"
#include "DSP/HarmonicsEngine.h"
#include "DSP/OversamplingEngine.h"
#include "DSP/DCBlocker.h"
#include "DSP/LevelMeter.h"
#include "DSP/VUMeterBallistics.h"
#include "Presets.h"

class RatosaurusProcessor final : public juce::AudioProcessor
{
public:
    RatosaurusProcessor();
    ~RatosaurusProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    float getCurrentGainReductionDb() const noexcept
    {
        return compressor.getCurrentGainReductionDb();
    }

    float getInputLevelDb()  const noexcept { return inputMeter.getLevelDb();  }
    float getOutputLevelDb() const noexcept { return outputMeter.getLevelDb(); }
    rato::VUMeterBallistics& getOutputVUMeterSource() noexcept { return outputVUMeter; }

    // --- Stage 8: Presets ---
    int  getNumPresets() const noexcept;
    juce::StringArray getPresetNames() const;
    void loadPreset(int index);
    int  getCurrentPresetIndex() const noexcept { return currentPresetIndex; }

    // --- Stage 9: A/B ---
    void toggleAB();
    void copyActiveToOther();
    int  getActiveSlot() const noexcept { return activeSlot; } // 0 = A, 1 = B

private:
    juce::AudioProcessorValueTreeState apvts;

    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;

    rato::CompressorEngine compressor;

    // Selectable FIR oversampling + harmonics + DC blocker
    rato::OversamplingEngine oversamplingEngine;
    rato::HarmonicsEngine harmonics;
    rato::DCBlocker dcBlocker;

    // Stage 7: peak meters (atomic-published, lock-free GUI access)
    rato::LevelMeter inputMeter;
    rato::LevelMeter outputMeter;
    rato::VUMeterBallistics outputVUMeter;

    // Stage 8: current preset index (-1 = none / user-modified)
    int currentPresetIndex = -1;

    // Stage 10E: host-side bypass parameter (cached pointer to APVTS param)
    juce::AudioParameterBool* bypassParam = nullptr;

    // Stage 9: A/B comparison state
    juce::ValueTree stateA;
    juce::ValueTree stateB;
    int activeSlot = 0;

    void updateCompressorParameters();
    void updateHarmonicsParameters();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RatosaurusProcessor)
};

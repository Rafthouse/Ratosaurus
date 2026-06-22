#include "PluginProcessor.h"
#include "PluginEditor.h"

RatosaurusProcessor::RatosaurusProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "RATOSAURUS_PARAMS", createParameterLayout())
{
    // Stage 9: both A/B slots start mirroring the default state.
    stateA = apvts.copyState();
    stateB = apvts.copyState();

    // Stage 10E: cache bypass parameter pointer for fast audio-thread access.
    bypassParam = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("bypass"));
    jassert(bypassParam != nullptr);
}

RatosaurusProcessor::~RatosaurusProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout RatosaurusProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- Stage 10E: Bypass (host-recognised) ---
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"bypass", 1},
        "Bypass",
        false));

    // --- Stage 1: Input / Output ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"inputGain", 1},
        "Input",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f, 2.5f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGain", 1},
        "Output",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f, 2.5f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // --- Stage 2: Compressor ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"threshold", 1},
        "Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -20.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"ratio", 1},
        "Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.4f),
        4.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"attack", 1},
        "Attack",
        juce::NormalisableRange<float>(0.05f, 50.0f, 0.01f, 0.3f),
        10.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"release", 1},
        "Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.35f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"detectorMode", 1},
        "Detector",
        juce::StringArray{"Peak", "RMS"},
        0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"knee", 1},
        "Knee",
        juce::StringArray{"Hard", "Medium", "Soft"},
        0));

    // --- Stage 3A: Character ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"characterMode", 1},
        "Character",
        juce::StringArray{"Clean", "Punch", "Rat"},
        0));

    // --- Stage 3B: Harmonics ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"drive", 1},
        "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"density", 1},
        "Density",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"color", 1},
        "Color",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f),
        0.0f));

    // --- Stage 4: Saturation Model ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"saturationModel", 1},
        "Saturation",
        juce::StringArray{"Off", "Tape", "Tube", "FET", "Rat"},
        0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"oversampling", 1},
        "Oversampling",
        juce::StringArray{"Off", "2x", "4x", "8x"},
        2));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"vuCalibration", 1},
        "VU Calibration",
        juce::StringArray{"-18 dBFS = 0 VU", "-24 dBFS = 0 VU", "-20 dBFS = 0 VU"},
        0));

    return layout;
}

void RatosaurusProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const auto numChannels = getTotalNumOutputChannels();

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(numChannels);

    inputGain.prepare(spec);
    outputGain.prepare(spec);

    inputGain.setRampDurationSeconds(0.02);
    outputGain.setRampDurationSeconds(0.02);

    compressor.prepare(sampleRate, samplesPerBlock, numChannels);

    const int initialOversampling = static_cast<int>(
        apvts.getRawParameterValue("oversampling")->load());
    oversamplingEngine.prepare(sampleRate, samplesPerBlock, numChannels,
                               initialOversampling);
    setLatencySamples(oversamplingEngine.getLatencySamples());

    juce::dsp::ProcessSpec harmonicsSpec{};
    harmonicsSpec.sampleRate = sampleRate * 8.0;
    harmonicsSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock * 8);
    harmonicsSpec.numChannels = static_cast<juce::uint32>(numChannels);
    harmonics.prepare(harmonicsSpec);

    dcBlocker.prepare(sampleRate, numChannels);
    dcBlocker.reset();

    inputMeter.prepare(sampleRate);
    outputMeter.prepare(sampleRate);
    outputVUMeter.prepare(sampleRate);
}

void RatosaurusProcessor::releaseResources()
{
    inputGain.reset();
    outputGain.reset();
    compressor.reset();
    harmonics.reset();
    dcBlocker.reset();
    inputMeter.reset();
    outputMeter.reset();
    outputVUMeter.reset();
    oversamplingEngine.reset();
}

bool RatosaurusProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == mainOut;
}

void RatosaurusProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numInputCh  = getTotalNumInputChannels();
    const auto numOutputCh = getTotalNumOutputChannels();

    for (auto i = numInputCh; i < numOutputCh; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Stage 10E: when bypassed, skip all DSP. Still pump meters with the dry
    // signal so the user sees what's coming in / going out. No allocations,
    // single atomic load on the audio thread.
    if (bypassParam != nullptr && bypassParam->get())
    {
        inputMeter.processBlock(buffer);
        outputMeter.processBlock(buffer);
        const int calibration = static_cast<int>(apvts.getRawParameterValue("vuCalibration")->load());
        outputVUMeter.processBlock(buffer,
            rato::VUMeterBallistics::calibrationOffsetForChoice(calibration));
        return;
    }

    inputGain.setGainDecibels(apvts.getRawParameterValue("inputGain")->load());
    outputGain.setGainDecibels(apvts.getRawParameterValue("outputGain")->load());

    updateCompressorParameters();
    updateHarmonicsParameters();

    auto audioBlock = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(audioBlock);

    inputGain.process(context);

    // Stage 7: input meter — measures post-input-gain signal (what compressor sees).
    inputMeter.processBlock(buffer);

    compressor.process(buffer);

    // Stage 3B: oversample 4x → harmonic shape → downsample → DC block.
    const int oversamplingMode = static_cast<int>(
        apvts.getRawParameterValue("oversampling")->load());
    oversamplingEngine.process(buffer, harmonics, oversamplingMode);
    dcBlocker.process(buffer);

    outputGain.process(context);

    // Stage 7: output meter — measures final post-output-gain signal.
    outputMeter.processBlock(buffer);
    const int calibration = static_cast<int>(apvts.getRawParameterValue("vuCalibration")->load());
    outputVUMeter.processBlock(buffer,
        rato::VUMeterBallistics::calibrationOffsetForChoice(calibration));
}

void RatosaurusProcessor::updateCompressorParameters()
{
    compressor.setThreshold(apvts.getRawParameterValue("threshold")->load());
    compressor.setRatio(apvts.getRawParameterValue("ratio")->load());
    compressor.setAttackMs(apvts.getRawParameterValue("attack")->load());
    compressor.setReleaseMs(apvts.getRawParameterValue("release")->load());

    const int detMode = static_cast<int>(apvts.getRawParameterValue("detectorMode")->load());
    compressor.setDetectorMode(static_cast<rato::DetectorMode>(detMode));

    const int kneeMode = static_cast<int>(apvts.getRawParameterValue("knee")->load());
    compressor.setKneeMode(static_cast<rato::KneeMode>(kneeMode));

    const int charMode = static_cast<int>(apvts.getRawParameterValue("characterMode")->load());
    compressor.setCharacterMode(static_cast<rato::CharacterMode>(charMode));
}

void RatosaurusProcessor::updateHarmonicsParameters()
{
    harmonics.setDrive  (apvts.getRawParameterValue("drive")  ->load());
    harmonics.setDensity(apvts.getRawParameterValue("density")->load());
    harmonics.setColor  (apvts.getRawParameterValue("color")  ->load());

    const int satModel = static_cast<int>(apvts.getRawParameterValue("saturationModel")->load());
    harmonics.setSaturationModel(static_cast<rato::SaturationModel>(satModel));
}

juce::AudioProcessorEditor* RatosaurusProcessor::createEditor()
{
    return new RatosaurusEditor(*this);
}

void RatosaurusProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("currentPresetIndex", currentPresetIndex, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RatosaurusProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        auto restored = juce::ValueTree::fromXml(*xml);
        currentPresetIndex = restored.getProperty("currentPresetIndex", -1);
        apvts.replaceState(restored);
    }
}

// ---------------------------------------------------------------------------
//  Stage 8: Presets
// ---------------------------------------------------------------------------
int RatosaurusProcessor::getNumPresets() const noexcept
{
    return static_cast<int>(rato::kFactoryPresets.size());
}

juce::StringArray RatosaurusProcessor::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& p : rato::kFactoryPresets)
        names.add(juce::String::fromUTF8(p.name));
    return names;
}

void RatosaurusProcessor::loadPreset(int index)
{
    if (index < 0 || index >= getNumPresets())
        return;

    const auto& preset = rato::kFactoryPresets[static_cast<size_t>(index)];

    for (const auto& pv : preset.params)
    {
        if (auto* parameter = apvts.getParameter(juce::String::fromUTF8(pv.id)))
        {
            const auto& range = parameter->getNormalisableRange();
            const float normalised = range.convertTo0to1(pv.value);
            parameter->setValueNotifyingHost(normalised);
        }
    }

    currentPresetIndex = index;
}

// ---------------------------------------------------------------------------
//  Stage 9: A/B comparison
// ---------------------------------------------------------------------------
void RatosaurusProcessor::toggleAB()
{
    // Save current APVTS state into the active slot before switching.
    if (activeSlot == 0)
        stateA = apvts.copyState();
    else
        stateB = apvts.copyState();

    activeSlot = 1 - activeSlot;

    const auto& nextState = (activeSlot == 0) ? stateA : stateB;
    if (nextState.isValid())
        apvts.replaceState(nextState);
}

void RatosaurusProcessor::copyActiveToOther()
{
    const auto current = apvts.copyState();
    if (activeSlot == 0)
        stateB = current;
    else
        stateA = current;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RatosaurusProcessor();
}

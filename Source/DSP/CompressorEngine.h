#pragma once

#include "PeakDetector.h"
#include "RMSDetector.h"
#include "GainComputer.h"
#include "CharacterEngine.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace rato
{

enum class DetectorMode { Peak = 0, RMS };
enum class KneeMode     { Hard = 0, Medium, Soft };

class CompressorEngine
{
public:
    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();
    void process(juce::AudioBuffer<float>& buffer);

    void setThreshold(float dB);
    void setRatio(float ratio);
    void setAttackMs(float ms);
    void setReleaseMs(float ms);
    void setDetectorMode(DetectorMode mode);
    void setKneeMode(KneeMode mode);
    void setCharacterMode(CharacterMode mode);

    float getCurrentGainReductionDb() const noexcept { return currentGainReductionDb; }

private:
    PeakDetector peakDetector;
    RMSDetector  rmsDetector;
    GainComputer gainComputer;

    DetectorMode detectorMode = DetectorMode::Peak;

    float currentGainReductionDb = 0.0f;

    CharacterEngine characterEngine;

    float detectLevel(float inputAbsolute);
};

} // namespace rato

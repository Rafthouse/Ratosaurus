#pragma once

#include <array>
#include <cstddef>

namespace rato
{

struct PresetParam
{
    const char* id;
    float value;
};

struct Preset
{
    const char* name;
    std::array<PresetParam, 13> params;
};

// Parameter index conventions across the table:
//   0  inputGain        dB
//   1  outputGain       dB
//   2  threshold        dB
//   3  ratio            x:1
//   4  attack           ms
//   5  release          ms
//   6  detectorMode     0 = Peak, 1 = RMS
//   7  knee             0 = Hard, 1 = Medium, 2 = Soft
//   8  characterMode    0 = Clean, 1 = Punch, 2 = Rat
//   9  drive            0..1
//  10  density          0..1
//  11  color            -1..1
//  12  saturationModel  0 = Off, 1 = Tape, 2 = Tube, 3 = FET, 4 = Rat

constexpr std::array<Preset, 13> kFactoryPresets = {{
    { "Default", {{
        { "inputGain",        0.0f }, { "outputGain",       0.0f },
        { "threshold",      -20.0f }, { "ratio",            4.0f },
        { "attack",          10.0f }, { "release",        100.0f },
        { "detectorMode",     0.0f }, { "knee",             0.0f },
        { "characterMode",    0.0f },
        { "drive",            0.0f }, { "density",          0.5f }, { "color", 0.0f },
        { "saturationModel",  0.0f }
    }}},

    { "Drum Smash", {{
        { "inputGain",        0.0f }, { "outputGain",       2.0f },
        { "threshold",      -18.0f }, { "ratio",            8.0f },
        { "attack",           1.0f }, { "release",         80.0f },
        { "detectorMode",     0.0f }, { "knee",             0.0f },
        { "characterMode",    2.0f },
        { "drive",            0.6f }, { "density",          0.6f }, { "color", 0.0f },
        { "saturationModel",  3.0f }
    }}},

    { "Vocal Glue", {{
        { "inputGain",        0.0f }, { "outputGain",       1.5f },
        { "threshold",      -16.0f }, { "ratio",            3.0f },
        { "attack",          15.0f }, { "release",        120.0f },
        { "detectorMode",     1.0f }, { "knee",             2.0f },
        { "characterMode",    0.0f },
        { "drive",           0.25f }, { "density",          0.5f }, { "color", 0.3f },
        { "saturationModel",  2.0f }
    }}},

    { "Bass Control", {{
        { "inputGain",        0.0f }, { "outputGain",       1.0f },
        { "threshold",      -14.0f }, { "ratio",            4.0f },
        { "attack",          25.0f }, { "release",        250.0f },
        { "detectorMode",     1.0f }, { "knee",             1.0f },
        { "characterMode",    1.0f },
        { "drive",            0.3f }, { "density",          0.4f }, { "color", 0.5f },
        { "saturationModel",  1.0f }
    }}},

    { "Parallel Rat", {{
        { "inputGain",        3.0f }, { "outputGain",      -3.0f },
        { "threshold",      -30.0f }, { "ratio",           12.0f },
        { "attack",           0.5f }, { "release",         50.0f },
        { "detectorMode",     0.0f }, { "knee",             0.0f },
        { "characterMode",    2.0f },
        { "drive",           0.75f }, { "density",          0.7f }, { "color", -0.2f },
        { "saturationModel",  4.0f }
    }}},

    { "Mix Bus", {{
        { "inputGain",        0.0f }, { "outputGain",       0.5f },
        { "threshold",      -10.0f }, { "ratio",            2.0f },
        { "attack",          30.0f }, { "release",        200.0f },
        { "detectorMode",     1.0f }, { "knee",             2.0f },
        { "characterMode",    0.0f },
        { "drive",           0.15f }, { "density",          0.5f }, { "color", 0.4f },
        { "saturationModel",  1.0f }
    }}},

    { "Tape Warmth", {{
        { "inputGain",        0.0f }, { "outputGain",       0.0f },
        { "threshold",        0.0f }, { "ratio",            1.1f },
        { "attack",          10.0f }, { "release",        100.0f },
        { "detectorMode",     0.0f }, { "knee",             0.0f },
        { "characterMode",    0.0f },
        { "drive",            0.5f }, { "density",          0.5f }, { "color", 0.6f },
        { "saturationModel",  1.0f }
    }}},

    { "FET Punch", {{
        { "inputGain",        1.0f }, { "outputGain",       1.0f },
        { "threshold",      -20.0f }, { "ratio",           10.0f },
        { "attack",           0.3f }, { "release",         60.0f },
        { "detectorMode",     0.0f }, { "knee",             0.0f },
        { "characterMode",    1.0f },
        { "drive",            0.5f }, { "density",          0.6f }, { "color", 0.0f },
        { "saturationModel",  3.0f }
    }}},

    { "Tube Vocal", {{
        { "inputGain",        0.0f }, { "outputGain",       1.0f },
        { "threshold",      -18.0f }, { "ratio",            3.0f },
        { "attack",          20.0f }, { "release",        150.0f },
        { "detectorMode",     1.0f }, { "knee",             2.0f },
        { "characterMode",    0.0f },
        { "drive",            0.4f }, { "density",          0.4f }, { "color", 0.7f },
        { "saturationModel",  2.0f }
    }}},

    { "Snare Crack", {{
        { "inputGain",        1.0f }, { "outputGain",       1.0f },
        { "threshold",      -22.0f }, { "ratio",            6.0f },
        { "attack",           0.3f }, { "release",         70.0f },
        { "detectorMode",     0.0f }, { "knee",             0.0f },
        { "characterMode",    1.0f },
        { "drive",            0.5f }, { "density",          0.7f }, { "color", 0.0f },
        { "saturationModel",  3.0f }
    }}},

    { "Acoustic Glue", {{
        { "inputGain",        0.0f }, { "outputGain",       0.5f },
        { "threshold",      -16.0f }, { "ratio",            2.5f },
        { "attack",          20.0f }, { "release",        180.0f },
        { "detectorMode",     1.0f }, { "knee",             2.0f },
        { "characterMode",    0.0f },
        { "drive",           0.15f }, { "density",          0.4f }, { "color", 0.5f },
        { "saturationModel",  2.0f }
    }}},

    { "Master Limit", {{
        { "inputGain",        0.0f }, { "outputGain",       0.0f },
        { "threshold",       -6.0f }, { "ratio",           20.0f },
        { "attack",           0.5f }, { "release",        100.0f },
        { "detectorMode",     0.0f }, { "knee",             0.0f },
        { "characterMode",    0.0f },
        { "drive",            0.1f }, { "density",          0.3f }, { "color", 0.0f },
        { "saturationModel",  0.0f }
    }}},

    { "Slow Squeeze", {{
        { "inputGain",        0.0f }, { "outputGain",       1.5f },
        { "threshold",      -24.0f }, { "ratio",            2.5f },
        { "attack",          40.0f }, { "release",        400.0f },
        { "detectorMode",     1.0f }, { "knee",             2.0f },
        { "characterMode",    1.0f },
        { "drive",            0.2f }, { "density",          0.5f }, { "color", 0.3f },
        { "saturationModel",  1.0f }
    }}}
}};

} // namespace rato

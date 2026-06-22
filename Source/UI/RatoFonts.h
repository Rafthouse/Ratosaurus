#pragma once

#include <juce_graphics/juce_graphics.h>

namespace rato
{

/**
    Bundled-font access for the Ratosaurus v4 visual identity.

    Three typefaces loaded once from BinaryData:
      • Oswald          — UI labels, knob names, section content
      • Pirata One      — gothic headers, brand wordmark, signature
      • Share Tech Mono — numeric readouts, dB scales, tech text

    All fonts are OFL-licensed and bundled, so rendering is identical on
    every host regardless of installed system fonts.
*/
struct Fonts
{
    // height = pixel height of the font. `weight` only affects Oswald.
    static juce::Font oswald(float height, bool bold = false);
    static juce::Font pirata(float height);
    static juce::Font mono(float height);
};

} // namespace rato

#include "RatoFonts.h"
#include "BinaryData.h"

namespace rato
{

namespace
{
    struct TypefaceStore
    {
        juce::Typeface::Ptr oswald;
        juce::Typeface::Ptr pirata;
        juce::Typeface::Ptr mono;

        TypefaceStore()
        {
            oswald = juce::Typeface::createSystemTypefaceFor(
                BinaryData::OswaldVariable_ttf, BinaryData::OswaldVariable_ttfSize);
            pirata = juce::Typeface::createSystemTypefaceFor(
                BinaryData::PirataOne_ttf, BinaryData::PirataOne_ttfSize);
            mono = juce::Typeface::createSystemTypefaceFor(
                BinaryData::ShareTechMono_ttf, BinaryData::ShareTechMono_ttfSize);
        }
    };

    TypefaceStore& store()
    {
        static TypefaceStore s;
        return s;
    }
}

juce::Font Fonts::oswald(float height, bool bold)
{
    juce::Font f(juce::FontOptions(store().oswald).withHeight(height));
    return bold ? f.boldened() : f;
}

juce::Font Fonts::pirata(float height)
{
    return juce::Font(juce::FontOptions(store().pirata).withHeight(height));
}

juce::Font Fonts::mono(float height)
{
    return juce::Font(juce::FontOptions(store().mono).withHeight(height));
}

} // namespace rato

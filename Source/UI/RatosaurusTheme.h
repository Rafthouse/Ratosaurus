#pragma once

#include <juce_graphics/juce_graphics.h>

namespace rato::theme
{

// ---------------------------------------------------------------------------
//  Palette — Shchur Edition
//  Three layers: chassis (purple steel), interior (graphite), accents (amber/brass).
// ---------------------------------------------------------------------------

namespace colour
{
    // ────────────────────────────────────────────────────────────────────
    //  POISONED CITY PALETTE — v4 per Ratosaurus Plugin v4.dc.html.
    //  Darker void, more muted brand purple, hotter accent magenta.
    // ────────────────────────────────────────────────────────────────────

    // Chassis / rack ears
    inline constexpr juce::uint32 chassisLight = 0xFF3A2A44;   // rack ear top
    inline constexpr juce::uint32 chassisBase  = 0xFF241830;   // rack ear bottom
    inline constexpr juce::uint32 chassisDeep  = 0xFF120C16;   // shadow rim

    // Inset panel recess
    inline constexpr juce::uint32 panelTop     = 0xFF2A1D32;   // panel raised
    inline constexpr juce::uint32 panelMid     = 0xFF1F1428;   // section bg / header tag
    inline constexpr juce::uint32 panelDeep    = 0xFF1B1222;   // panel deep

    // Black recesses & meter wells
    inline constexpr juce::uint32 wellBlack    = 0xFF0A070C;   // void (darker in v4)
    inline constexpr juce::uint32 meterWell    = 0xFF0C0710;   // meter window

    // Purple glow — v4 brand signal: muted purple + hotter magenta
    inline constexpr juce::uint32 amberHigh    = 0xFFFF8DF5;   // hot magenta (active LED)
    inline constexpr juce::uint32 amberMid     = 0xFFCF3FD8;   // magenta active
    inline constexpr juce::uint32 amberLow     = 0xFFB06FD0;   // brand purple
    inline constexpr juce::uint32 amberDeep    = 0xFF7C4F96;   // purple deep

    // Brass / bronze trim — engraved text, dividers, bezels
    inline constexpr juce::uint32 brassBright  = 0xFFC2B291;   // knob brass top
    inline constexpr juce::uint32 brassPatina  = 0xFFA08A5E;   // bronze trim
    inline constexpr juce::uint32 brassDark    = 0xFF4A4030;   // knob brass deep

    // Chrome / metal highlights (corner bolts)
    inline constexpr juce::uint32 chromeHigh   = 0xFFCDBFD6;   // light cream
    inline constexpr juce::uint32 chromeMid    = 0xFF8F7F60;   // brass mid
    inline constexpr juce::uint32 chromeDeep   = 0xFF5A4A3A;   // brass shadow

    // Bone / fossil — for skull centerpiece (Stage 10C asset)
    inline constexpr juce::uint32 boneLight    = 0xFFD8C9A8;   // cream text
    inline constexpr juce::uint32 boneMid      = 0xFFA08A5E;   // bronze
    inline constexpr juce::uint32 boneDeep     = 0xFF4A2F24;   // rust
    inline constexpr juce::uint32 boneCrack    = 0xFF2A1525;   // panel shadow

    // Level meter gradient (v4: dim segment + magenta hot peak)
    inline constexpr juce::uint32 levelLow     = 0xFF7C4F96;   // purple
    inline constexpr juce::uint32 levelMid     = 0xFFB06FD0;   // brand purple
    inline constexpr juce::uint32 levelHigh    = 0xFFCF3FD8;   // magenta active
    inline constexpr juce::uint32 levelPeak    = 0xFFFF8DF5;   // magenta hot

    // GR meter fill (v4 magenta)
    inline constexpr juce::uint32 grFill       = 0xFFCF3FD8;

    // Knob metal — Soviet Fluted body, olive-black phenolic
    inline constexpr juce::uint32 knobOuter    = 0xFF221C18;   // fluted ridge
    inline constexpr juce::uint32 knobBase     = 0xFF0E0B0A;   // fluted groove
    inline constexpr juce::uint32 knobInner    = 0xFF1A1810;   // dome base
    inline constexpr juce::uint32 knobCapHigh  = 0xFF3A3A2C;   // dome top
    inline constexpr juce::uint32 knobCapDeep  = 0xFF0C0C08;   // dome center

    // LCD readouts (LED-style display)
    inline constexpr juce::uint32 lcdBgTop     = 0xFF241525;   // panel
    inline constexpr juce::uint32 lcdBgBottom  = 0xFF160D18;   // panel deep
    inline constexpr juce::uint32 lcdText      = 0xFFD8C9A8;   // cream readout
    inline constexpr juce::uint32 lcdBorder    = 0xFFA868C4;   // purple bright

    // Active saturation station (LED-glass tile)
    inline constexpr juce::uint32 stationLit   = 0xFFE87BF5;   // magenta hot
    inline constexpr juce::uint32 stationDim   = 0xFF1A1018;   // dark glass
    inline constexpr juce::uint32 stationFrame = 0xFFA868C4;   // purple bright
    inline constexpr juce::uint32 stationDark  = 0xFFA08A5E;   // bronze inactive

    // ─── Soviet Fluted knob — extras for layered paint ──────────────────
    inline constexpr juce::uint32 knobPointerHigh = 0xFFD8D0B0;   // engraved cream
    inline constexpr juce::uint32 knobPointerLow  = 0xFF8A8060;   // engraved gold
    inline constexpr juce::uint32 knobPointerHot  = 0xFFE87BF5;   // magenta active
    inline constexpr juce::uint32 knobHaloHover   = 0xFFCF57E0;   // hover ring
}

// ---------------------------------------------------------------------------
//  Typography
// ---------------------------------------------------------------------------
namespace font
{
    // Sizes (in pt)
    inline constexpr float brandTitle      = 26.0f;
    inline constexpr float brandSubtitle   = 10.0f;
    inline constexpr float sectionHeader   = 10.0f;
    inline constexpr float engravedLabel   = 11.5f;
    inline constexpr float lcdReadout      = 14.0f;
    inline constexpr float signatureMark   = 18.0f;
    inline constexpr float versionMark     = 8.0f;
    inline constexpr float knobValueText   = 11.0f;

    // Letter spacing (kerning)
    inline constexpr int   brandSpacing    = 14;
    inline constexpr int   subtitleSpacing = 6;
    inline constexpr int   sectionSpacing  = 5;
    inline constexpr int   engravedSpacing = 2;
}

// ---------------------------------------------------------------------------
//  Layout — geometry constants
// ---------------------------------------------------------------------------
namespace layout
{
    inline constexpr int windowWidth         = 720;
    inline constexpr int windowHeight        = 560;

    inline constexpr int chassisPadding      = 6;
    inline constexpr int panelPadding        = 16;
    inline constexpr int titleHeight         = 60;
    inline constexpr int topBarHeight        = 34;
    inline constexpr int sectionHeader       = 22;
    inline constexpr int knobRowHeight       = 100;
    inline constexpr int sectionGap          = 8;

    inline constexpr int largeKnobSize       = 80;
    inline constexpr int compactKnobSize     = 56;
    inline constexpr int knobLabelGap        = 4;

    inline constexpr int meterWidth          = 14;
    inline constexpr int meterGap            = 6;
    inline constexpr int grMeterWidth        = 22;

    inline constexpr int comboWidth          = 110;
    inline constexpr int comboHeight         = 26;

    inline constexpr int cornerBoltRadius    = 7;
    inline constexpr int cornerBoltInset     = 18;

    inline constexpr int emblemRadius        = 62;
    inline constexpr int emblemBoltCount     = 12;

    inline constexpr int stationCellWidth    = 52;
    inline constexpr int stationCellHeight   = 60;
    inline constexpr int stationIconSize     = 40;
}

// ---------------------------------------------------------------------------
//  Animation periods (milliseconds)
// ---------------------------------------------------------------------------
namespace animation
{
    inline constexpr int refreshHz           = 30;

    inline constexpr int eyePulsePeriodMs    = 3000;
    inline constexpr int emblemBreathMs      = 4500;
    inline constexpr int tapeReelPeriodMs    = 2000;
    inline constexpr int tubeGlowPeriodMs    = 2500;
    inline constexpr int fetLedPeriodMs      = 1800;

    inline constexpr int modeSwitchFadeMs    = 200;
}

// ---------------------------------------------------------------------------
//  Helper
// ---------------------------------------------------------------------------
inline juce::Colour col(juce::uint32 hex) noexcept
{
    return juce::Colour(hex);
}

} // namespace rato::theme

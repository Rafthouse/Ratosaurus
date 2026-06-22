#include "PluginEditor.h"
#include "UI/RatosaurusTheme.h"
#include "UI/RatoFonts.h"
#include "BinaryData.h"

namespace
{
    // v4 base canvas (per Ratosaurus Plugin v4.dc.html — 1400×620 rack format)
    constexpr int kWindowWidth    = 1400;
    constexpr int kWindowHeight   = 720;

    // Rack ears + chassis
    constexpr int kRackEarWidth   = 34;
    constexpr int kChassisPadding = 14;
    constexpr int kPanelInsetX    = 38;        // inset from rack ears
    constexpr int kPanelPaddingY  = 12;

    // Section heights
    constexpr int kSectionGap     = 10;
    constexpr int kSectionHeaderH = 22;        // floating header tag height
    constexpr int kIoSectionH     = 150;
    constexpr int kCompSectionH   = 160;
    constexpr int kSatSectionH    = 150;
    constexpr int kBottomBarH     = 56;

    // Layout primitives (kept some for shared helpers)
    constexpr int kPadding        = 14;
    constexpr int kTitleHeight    = 40;
    constexpr int kTopBarHeight   = 30;
    constexpr int kFooterHeight   = 26;
    constexpr int kHeaderHeight   = 20;
    constexpr int kKnobRow        = 130;       // v4 row height
    constexpr int kComboRowOnly   = 60;
    constexpr int kLabelHeight    = 18;

    // Knobs (v4 sizes)
    constexpr int kKnobWidth      = 118;       // I/O
    constexpr int kCompKnobWidth  = 110;
    constexpr int kSatKnobWidth   = 104;
    constexpr int kComboWidth     = 130;
    constexpr int kComboHeight    = 28;

    constexpr int kMeterWidth     = 22;
    constexpr int kMeterGap       = 6;
    constexpr int kGrMeterWidth   = 22;

    constexpr int kLogoMaxWidth   = 260;
    constexpr int kSignatureWidth = 240;
    constexpr int kSatIconWidth   = 56;

    constexpr int kRefreshHz      = 30;

    // v4 palette
    const juce::Colour kBackground { 0xFF0A070C };   // void
    const juce::Colour kChassisHi  { 0xFF1C1320 };   // outer chassis top
    const juce::Colour kChassisLo  { 0xFF120C16 };   // outer chassis bottom
    const juce::Colour kRackEar    { 0xFF3A2A44 };   // rack ear top
    const juce::Colour kRackEarLo  { 0xFF241830 };   // rack ear bottom
    const juce::Colour kInnerPanel { 0xFF2A1D32 };   // inner panel top
    const juce::Colour kInnerPanLo { 0xFF1B1222 };   // inner panel bottom
    const juce::Colour kPanel      { 0xFF1F1428 };   // section bg / header tag
    const juce::Colour kSectionBorder { 0x47B06FD0 };// border 28% alpha
    const juce::Colour kBrandPurple{ 0xFFB06FD0 };   // brand purple
    const juce::Colour kMagentaHot { 0xFFFF8DF5 };   // hot magenta
    const juce::Colour kTextCream  { 0xFFC9B8A0 };   // primary cream text
    const juce::Colour kTextMuted  { 0xFF9A8AA6 };   // dim text
    const juce::Colour kTextDim    { 0xFF7A6A86 };   // very dim
    const juce::Colour kAccent     { 0xFFC9B8A0 };   // accent (same as cream)
    const juce::Colour kHeader     { 0xFFC9B8A0 };   // header text
}

// =========================================================================
//  RatosaurusEditor
// =========================================================================

RatosaurusEditor::RatosaurusEditor(RatosaurusProcessor& p)
    : AudioProcessorEditor(p),
      processor(p),
      vuMeter(processor.getOutputVUMeterSource(), processor.getAPVTS())
{
    setLookAndFeel(&lookAndFeel);

    // v5: load the v4 faceplate render (BinaryData). Window size + aspect
    // derive from it so controls map 1:1 onto the artwork.
    auto& assets = rato::AssetCache::getInstance();
    faceplateDrawable = assets.createDrawable(rato::AssetCache::Asset::Faceplate);
    setSize(kWindowWidth, kWindowHeight);

    setResizable(true, true);
    if (auto* c = getConstrainer())
    {
        c->setMinimumSize(840, 432);
        c->setMaximumSize(2240, 1152);
        c->setFixedAspectRatio(1400.0 / 720.0);
    }

    // --- Stage 10D: SVG assets ---
    logoDrawable      = assets.createDrawable(rato::AssetCache::Asset::Logo);
    signatureDrawable = assets.createDrawable(rato::AssetCache::Asset::Signature);

    // --- Top bar: presets + A/B ---
    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setFont(rato::Fonts::oswald(13.5f, true));
    presetLabel.setColour(juce::Label::textColourId, kHeader);
    addAndMakeVisible(presetLabel);

    presetCombo.addItemList(processor.getPresetNames(), 1);
    presetCombo.setTextWhenNothingSelected("SELECT PRESET");
    presetCombo.onChange = [this]()
    {
        const int selectedId = presetCombo.getSelectedId();
        if (selectedId <= 0)
            return;
        const int presetIndex = selectedId - 1;
        if (presetIndex != processor.getCurrentPresetIndex())
            processor.loadPreset(presetIndex);
    };
    addAndMakeVisible(presetCombo);

    aButton.setButtonText("A");
    bButton.setButtonText("B");
    copyButton.setButtonText("Copy");
    bypassButton.setButtonText("BYPASS");
    bypassButton.setClickingTogglesState(true);

    aButton.onClick = [this]()
    {
        if (processor.getActiveSlot() != 0)
        {
            processor.toggleAB();
            refreshAbButtons();
        }
    };
    bButton.onClick = [this]()
    {
        if (processor.getActiveSlot() != 1)
        {
            processor.toggleAB();
            refreshAbButtons();
        }
    };
    copyButton.onClick = [this]()
    {
        processor.copyActiveToOther();
    };

    for (auto* b : { &aButton, &bButton, &copyButton })
    {
        b->setColour(juce::TextButton::textColourOffId, kAccent);
        b->setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
        addAndMakeVisible(*b);
    }

    // Stage 10E: bypass — bright magenta when active, dim purple when off.
    bypassButton.setColour(juce::TextButton::textColourOffId,   juce::Colour(0xFF9A7BB0));
    bypassButton.setColour(juce::TextButton::textColourOnId,    juce::Colour(0xFF0D070F));
    bypassButton.setColour(juce::TextButton::buttonColourId,    juce::Colour(0xFF1A1018));
    bypassButton.setColour(juce::TextButton::buttonOnColourId,  juce::Colour(0xFFCF57E0));
    bypassButton.setAlpha(0.01f); // transparent hit target; v4 SVG paints the hardware label/LED
    bypassButton.onStateChange = [this] { repaint(); };
    addAndMakeVisible(bypassButton);

    refreshPresetCombo();
    refreshAbButtons();

    // --- I/O ---
    // Brushed Metal per Knob Build.dc.html — exact 10-step spec.
    inputKnob .setStyle(rato::KnobStyle::BrushedAlu);
    outputKnob.setStyle(rato::KnobStyle::BrushedAlu);
    setupKnob(inputKnob,  inputLabel,  "Input");
    setupKnob(outputKnob, outputLabel, "Output");

    inputMeterUI.setMode(rato::RatoMeter::Mode::Level);
    outputMeterUI.setMode(rato::RatoMeter::Mode::Level);
    addAndMakeVisible(inputMeterUI);
    addAndMakeVisible(outputMeterUI);
    addAndMakeVisible(vuMeter);

    // --- Compressor ---
    // All knobs use the Knob Build.dc.html spec (Brushed Metal #2).
    thresholdKnob.setStyle(rato::KnobStyle::BrushedAlu);
    ratioKnob    .setStyle(rato::KnobStyle::BrushedAlu);
    attackKnob   .setStyle(rato::KnobStyle::BrushedAlu);
    releaseKnob  .setStyle(rato::KnobStyle::BrushedAlu);
    setupKnob(thresholdKnob, thresholdLabel, "Threshold");
    setupKnob(ratioKnob,     ratioLabel,     "Ratio");
    setupKnob(attackKnob,    attackLabel,    "Attack");
    setupKnob(releaseKnob,   releaseLabel,   "Release");

    // v4 R1: KNEE / DETECTOR as LED radio selectors instead of ComboBoxes
    kneeSelector = std::make_unique<rato::RatoLedSelector>(
        processor.getAPVTS(), "knee", "KNEE",
        juce::StringArray{ "HARD", "MED", "SOFT" });
    addAndMakeVisible(*kneeSelector);

    detectorSelector = std::make_unique<rato::RatoLedSelector>(
        processor.getAPVTS(), "detectorMode", "DETECTOR",
        juce::StringArray{ "PEAK", "RMS" });
    addAndMakeVisible(*detectorSelector);

    grMeterUI.setMode(rato::RatoMeter::Mode::GainReduction);
    addAndMakeVisible(grMeterUI);

    grLabel.setText("GR", juce::dontSendNotification);
    grLabel.setJustificationType(juce::Justification::centred);
    grLabel.setFont(rato::Fonts::oswald(11.0f, true));
    grLabel.setColour(juce::Label::textColourId, kHeader);
    addAndMakeVisible(grLabel);

    // --- Character (v4 R1: LED radio selector) ---
    characterSelector = std::make_unique<rato::RatoLedSelector>(
        processor.getAPVTS(), "characterMode", "CHARACTER",
        juce::StringArray{ "VCA", "OPTO", "RAT" });
    addAndMakeVisible(*characterSelector);

    // --- Harmonics + Saturation ---
    driveKnob  .setStyle(rato::KnobStyle::BrushedAlu);
    densityKnob.setStyle(rato::KnobStyle::BrushedAlu);
    colorKnob  .setStyle(rato::KnobStyle::BrushedAlu);
    setupKnob(driveKnob,   driveLabel,   "Drive");
    setupKnob(densityKnob, densityLabel, "Density");
    setupKnob(colorKnob,   colorLabel,   "Color");

    // v4 R3: Saturation Model as 5 illuminated tiles (OFF/TAPE/TUBE/FET/RAT)
    satTiles = std::make_unique<rato::RatoSatTiles>(processor.getAPVTS(), "saturationModel");
    addAndMakeVisible(*satTiles);

    oversamplingCombo.addItemList(juce::StringArray{ "OFF", "2X", "4X", "8X" }, 1);
    oversamplingCombo.setTextWhenNothingSelected("4X");
    addAndMakeVisible(oversamplingCombo);

    // --- Section headers ---
    setupSectionHeader(sectionIO,   "I/O");
    setupSectionHeader(sectionComp, "COMPRESSOR");
    setupSectionHeader(sectionChar, "CHARACTER");
    setupSectionHeader(sectionSat,  "SATURATION");

    // --- APVTS attachments ---
    auto& apvts = processor.getAPVTS();

    inputAtt     = std::make_unique<SliderAttachment>  (apvts, "inputGain",       inputKnob);
    outputAtt    = std::make_unique<SliderAttachment>  (apvts, "outputGain",      outputKnob);

    thresholdAtt = std::make_unique<SliderAttachment>  (apvts, "threshold",       thresholdKnob);
    ratioAtt     = std::make_unique<SliderAttachment>  (apvts, "ratio",           ratioKnob);
    attackAtt    = std::make_unique<SliderAttachment>  (apvts, "attack",          attackKnob);
    releaseAtt   = std::make_unique<SliderAttachment>  (apvts, "release",         releaseKnob);

    // knee / detectorMode / characterMode are now owned by the RatoLedSelectors
    // (they bind their own ParameterAttachment internally).

    driveAtt     = std::make_unique<SliderAttachment>  (apvts, "drive",           driveKnob);
    densityAtt   = std::make_unique<SliderAttachment>  (apvts, "density",         densityKnob);
    colorAtt     = std::make_unique<SliderAttachment>  (apvts, "color",           colorKnob);

    // saturationModel now owned by satTiles (binds its own ParameterAttachment).
    bypassAtt     = std::make_unique<ButtonAttachment>  (apvts, "bypass",          bypassButton);
    oversamplingAtt = std::make_unique<ComboBoxAttachment>(apvts, "oversampling", oversamplingCombo);

    updateSaturationIcon(); // initial state matches APVTS

    // The first resized() fired from setSize() in this ctor ran BEFORE the
    // unique_ptr child components (LED selectors) existed, so they never got
    // bounds. Re-run layout now that every component is constructed.
    resized();

    // v5 milestone: faceplate-only verification — hide every live control so
    // the screenshot shows the pure v4 artwork. Controls get re-enabled and
    // positioned onto the faceplate coordinates in the next step.
    startTimerHz(kRefreshHz);
}

RatosaurusEditor::~RatosaurusEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void RatosaurusEditor::timerCallback()
{
    inputMeterUI.setLevelDb (processor.getInputLevelDb());
    outputMeterUI.setLevelDb(processor.getOutputLevelDb());
    grMeterUI.setLevelDb    (processor.getCurrentGainReductionDb());
    vuMeter.updateDisplay();

    refreshPresetCombo();
    refreshAbButtons();

    // Stage 10D: detect saturationModel changes (preset load, A/B toggle, manual).
    if (auto* satParam = processor.getAPVTS().getRawParameterValue("saturationModel"))
    {
        const int satIdx = static_cast<int>(satParam->load());
        if (satIdx != lastSatModelIndex)
            updateSaturationIcon();
    }

    // Stage 10E: tick the animation clock and trigger a tight repaint on the
    // sat icon zone (breathing pulse) only — knob hovers repaint themselves
    // via Slider's own mouse-enter/leave callbacks.
    animationTimeSec += 1.0 / 30.0;
    if (! satIconBounds.isEmpty())
        repaint(satIconBounds);
}

void RatosaurusEditor::refreshPresetCombo()
{
    const int idx = processor.getCurrentPresetIndex();
    const int currentSelection = presetCombo.getSelectedId() - 1;
    if (idx != currentSelection)
        presetCombo.setSelectedId(idx + 1, juce::dontSendNotification);
}

void RatosaurusEditor::refreshAbButtons()
{
    const juce::Colour active   { 0xFFFFA940 };
    const juce::Colour inactive { 0xFF3A3A3A };
    const int slot = processor.getActiveSlot();
    aButton.setColour(juce::TextButton::buttonColourId, slot == 0 ? active : inactive);
    bButton.setColour(juce::TextButton::buttonColourId, slot == 1 ? active : inactive);
}

void RatosaurusEditor::updateSaturationIcon()
{
    auto* satParam = processor.getAPVTS().getRawParameterValue("saturationModel");
    const int satIdx = satParam != nullptr ? static_cast<int>(satParam->load()) : 0;
    lastSatModelIndex = satIdx;

    using Asset = rato::AssetCache::Asset;
    Asset key = Asset::IconOff;
    switch (satIdx)
    {
        case 0: key = Asset::IconOff;  break;
        case 1: key = Asset::IconTape; break;
        case 2: key = Asset::IconTube; break;
        case 3: key = Asset::IconFet;  break;
        case 4: key = Asset::IconRat;  break;
        default: key = Asset::IconOff; break;
    }

    currentSatIcon = rato::AssetCache::getInstance().createDrawable(key);

    if (! satIconBounds.isEmpty())
        repaint(satIconBounds);
}

void RatosaurusEditor::setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& text)
{
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    knob.onValueChange = [this] { repaint(); };
    addAndMakeVisible(knob);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(rato::Fonts::oswald(12.0f, true));
    label.setColour(juce::Label::textColourId, kHeader);
    addAndMakeVisible(label);
}

void RatosaurusEditor::setupCombo(juce::ComboBox& combo, juce::Label& label,
                                  const juce::String& text, const juce::StringArray& items)
{
    combo.addItemList(items, 1);
    addAndMakeVisible(combo);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(rato::Fonts::oswald(12.0f, true));
    label.setColour(juce::Label::textColourId, kHeader);
    addAndMakeVisible(label);
}

void RatosaurusEditor::setupSectionHeader(juce::Label& header, const juce::String& text)
{
    header.setText(text, juce::dontSendNotification);
    header.setJustificationType(juce::Justification::centredLeft);
    header.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    header.setColour(juce::Label::textColourId, kHeader);
    addAndMakeVisible(header);
}

void RatosaurusEditor::paint(juce::Graphics& g)
{
    // v5 faceplate architecture: the v4 render IS the static art. Draw it to
    // fill the editor; native controls overlay on top. All previously
    // hand-painted chrome below is bypassed.
    if (faceplateDrawable != nullptr)
    {
        faceplateDrawable->drawWithin(g, getLocalBounds().toFloat(),
                                      juce::RectanglePlacement::stretchToFit, 1.0f);
        if (logoDrawable != nullptr && ! logoBounds.isEmpty())
            logoDrawable->drawWithin(g, logoBounds.toFloat(),
                                     juce::RectanglePlacement::centred, 1.0f);

        const float uiScale = static_cast<float>(getHeight()) / 720.0f;
        auto scaledRect = [uiScale](int x, int y, int w, int h)
        {
            return juce::Rectangle<int>(static_cast<int>(std::round(x * uiScale)),
                                        static_cast<int>(std::round(y * uiScale)),
                                        static_cast<int>(std::round(w * uiScale)),
                                        static_cast<int>(std::round(h * uiScale)));
        };

        auto drawTag = [&](juce::Rectangle<int> area)
        {
            const auto r = area.toFloat();
            g.setColour(juce::Colour(0xFF0C0710));
            g.fillRoundedRectangle(r, 3.0f * uiScale);
            g.setColour(juce::Colour(0xFF7C4F96).withAlpha(0.65f));
            g.drawRoundedRectangle(r, 3.0f * uiScale, 1.0f);
        };

        drawTag(scaledRect(86, 20, 62, 30));
        drawTag(scaledRect(86, 254, 180, 31));
        drawTag(scaledRect(86, 456, 154, 31));

        g.setColour(kBrandPurple);
        g.setFont(rato::Fonts::pirata(22.0f * uiScale));
        g.drawText("I/O",        scaledRect(98, 20, 100, 30), juce::Justification::centredLeft);
        g.drawText("COMPRESSOR", scaledRect(98, 254, 190, 30), juce::Justification::centredLeft);
        g.drawText("SATURATION", scaledRect(98, 456, 170, 30), juce::Justification::centredLeft);

        const auto brandPlate = scaledRect(1050, 174, 230, 62).toFloat();
        g.setColour(juce::Colour(0xFF0C0710).withAlpha(0.94f));
        g.fillRoundedRectangle(brandPlate, 4.0f * uiScale);
        g.setColour(juce::Colour(0xFF7C4F96).withAlpha(0.55f));
        g.drawRoundedRectangle(brandPlate, 4.0f * uiScale, 1.0f);
        g.setColour(kBrandPurple);
        g.setFont(rato::Fonts::pirata(30.0f * uiScale));
        g.drawFittedText("RATOSAURUS", scaledRect(1058, 177, 214, 38),
                         juce::Justification::centred, 1, 0.88f);
        g.setColour(kTextCream);
        g.setFont(rato::Fonts::oswald(11.0f * uiScale));
        g.drawFittedText("COMPRESSOR / DISTRESSOR", scaledRect(1058, 211, 214, 18),
                         juce::Justification::centred, 1);

        // Remove the static zero markers from the SVG and replace them with
        // live parameter readouts below every knob.
        g.setColour(juce::Colour(0xFF1A1120));
        g.fillRect(scaledRect(146, 67, 28, 14));
        g.fillRect(scaledRect(376, 67, 28, 14));

        auto drawReadout = [&](const juce::String& text, juce::Rectangle<int> area)
        {
            g.setColour(juce::Colour(0xFFD8C9A8));
            g.setFont(rato::Fonts::mono(12.0f * uiScale));
            g.drawFittedText(text, area, juce::Justification::centred, 1, 0.82f);
        };
        auto formatDb = [](double value)
        {
            return value <= -59.95 ? juce::String("-inf dB")
                                   : juce::String(value, 1) + " dB";
        };
        auto formatMs = [](double value)
        {
            return value < 1.0 ? juce::String(value, 2) : juce::String(value, 1);
        };

        drawReadout(formatDb(inputKnob.getValue()),       scaledRect(118, 194, 84, 18));
        drawReadout(formatDb(outputKnob.getValue()),      scaledRect(348, 194, 84, 18));
        drawReadout(juce::String(thresholdKnob.getValue(), 1) + " dB",
                    scaledRect(112, 414, 88, 18));
        drawReadout(juce::String(ratioKnob.getValue(), ratioKnob.getValue() < 10.0 ? 1 : 0) + ":1",
                    scaledRect(262, 414, 80, 18));
        drawReadout(formatMs(attackKnob.getValue()),      scaledRect(420, 414, 56, 18));
        drawReadout(formatMs(releaseKnob.getValue()),     scaledRect(560, 414, 68, 18));
        drawReadout(juce::String(driveKnob.getValue(), 2),   scaledRect(124, 612, 54, 18));
        drawReadout(juce::String(densityKnob.getValue(), 2), scaledRect(284, 612, 54, 18));
        drawReadout(juce::String(colorKnob.getValue(), 2),   scaledRect(444, 612, 54, 18));

        // Decorative ventilation around VU. Drive controls only the glow;
        // it never feeds back into DSP or meter calculations.
        const float driveGlow = static_cast<float>(juce::jlimit(0.0, 1.0, driveKnob.getValue()));
        auto drawVent = [&](juce::Rectangle<int> area)
        {
            const auto r = area.toFloat();
            g.setColour(juce::Colour(0xFF08060A));
            g.fillRoundedRectangle(r, 3.0f * uiScale);
            g.setColour(juce::Colour(0xFFCF3FD8).withAlpha(0.10f + driveGlow * 0.34f));
            g.drawRoundedRectangle(r, 3.0f * uiScale, 1.0f + driveGlow * uiScale);
            const int slots = 8;
            for (int slot = 0; slot < slots; ++slot)
            {
                const float y = r.getY() + (slot + 1.0f) * r.getHeight() / (slots + 1.0f);
                g.setColour(juce::Colour(0xFF2A1830).interpolatedWith(
                    juce::Colour(0xFFCF3FD8), driveGlow * 0.55f));
                g.drawLine(r.getX() + 5.0f * uiScale, y,
                           r.getRight() - 5.0f * uiScale, y,
                           juce::jmax(1.0f, 1.3f * uiScale));
            }
        };
        // Reference composition: equal 10 px air gaps around the live VU.
        drawVent(scaledRect(706, 78, 28, 142));
        drawVent(scaledRect(998, 78, 28, 142));
        g.setColour(juce::Colour(0xFF1B1222));
        g.fillRect(scaledRect(520, 660, 650, 44));

        const auto leftPlate  = scaledRect(558, 666, 142, 32).toFloat();
        const auto signature  = scaledRect(706, 663, 266, 38).toFloat();
        const auto rightPlate = scaledRect(978, 666, 176, 32).toFloat();

        g.setColour(juce::Colour(0xFF170F1C));
        g.fillRoundedRectangle(leftPlate, 4.0f * uiScale);
        g.fillRect(signature);
        g.fillRoundedRectangle(rightPlate, 4.0f * uiScale);
        g.setColour(juce::Colour(0xFF7C4F96).withAlpha(0.45f));
        g.drawRoundedRectangle(leftPlate, 4.0f * uiScale, 1.0f);
        g.drawRoundedRectangle(rightPlate, 4.0f * uiScale, 1.0f);

        g.setColour(juce::Colour(0xFF8C7A96));
        g.setFont(rato::Fonts::oswald(11.5f * uiScale, true));
        g.drawFittedText("MADE IN\nUKRAINE", leftPlate.toNearestInt(),
                         juce::Justification::centred, 2);
        g.drawFittedText("THE HEALING MUD\nFOR YOUR SOUND", rightPlate.toNearestInt(),
                         juce::Justification::centred, 2);

        g.setColour(juce::Colour(0xFFB06FD0));
        g.setFont(rato::Fonts::pirata(25.0f * uiScale));
        g.drawText("By Mykyta Shchur", signature.toNearestInt(),
                   juce::Justification::centred);

        g.setColour(juce::Colour(0xFF8C7A96));
        g.setFont(rato::Fonts::oswald(8.0f * uiScale, true));
        g.drawText("OS", scaledRect(1158, 658, 48, 10),
                   juce::Justification::centred, false);
        if (bypassButton.getToggleState())
        {
            const auto led = scaledRect(1296, 676, 14, 14).toFloat();
            g.setColour(kMagentaHot.withAlpha(0.28f));
            g.fillEllipse(led.expanded(5.0f * uiScale));
            g.setColour(kMagentaHot);
            g.fillEllipse(led);
        }
        return;
    }

    g.fillAll(kBackground);
#if 0
    const float scale = static_cast<float>(getHeight()) / static_cast<float>(kWindowHeight);
    const auto fullBoundsF = getLocalBounds().toFloat();

    g.fillAll(kBackground);

    // ── 1. Outer chassis (purple gradient) with subtle outer glow ─────────
    {
        juce::ColourGradient chassis(kChassisHi,
                                     fullBoundsF.getCentreX(), fullBoundsF.getY(),
                                     kChassisLo,
                                     fullBoundsF.getCentreX(), fullBoundsF.getBottom(),
                                     false);
        g.setGradientFill(chassis);
        g.fillRoundedRectangle(fullBoundsF, 14.0f * scale);
        g.setColour(kBrandPurple.withAlpha(0.25f));
        g.drawRoundedRectangle(fullBoundsF, 14.0f * scale, 1.0f);
    }

    // ── 2. Inner panel with purple ambient backdrop ───────────────────────
    const float earW = kRackEarWidth * scale;
    const float chassisPad = kChassisPadding * scale;
    auto innerArea = fullBoundsF.reduced(chassisPad);
    innerArea = innerArea.withTrimmedLeft(earW).withTrimmedRight(earW);
    {
        juce::ColourGradient panel(kInnerPanel,
                                   innerArea.getCentreX(), innerArea.getY(),
                                   kInnerPanLo,
                                   innerArea.getCentreX(), innerArea.getBottom(),
                                   false);
        g.setGradientFill(panel);
        g.fillRoundedRectangle(innerArea, 8.0f * scale);

        // Purple ambient glow from top
        juce::ColourGradient amb(kBrandPurple.withAlpha(0.16f),
                                 innerArea.getCentreX(), innerArea.getY(),
                                 juce::Colours::transparentBlack,
                                 innerArea.getCentreX(), innerArea.getY() + innerArea.getHeight() * 0.55f,
                                 false);
        g.setGradientFill(amb);
        g.fillRoundedRectangle(innerArea, 8.0f * scale);

        // Inner border
        g.setColour(kBrandPurple.withAlpha(0.22f));
        g.drawRoundedRectangle(innerArea, 8.0f * scale, 1.0f);
    }

    // ── 3. Rack ears (left + right) ───────────────────────────────────────
    auto outer = fullBoundsF.reduced(chassisPad);
    auto leftEar  = juce::Rectangle<float>(outer.getX(), outer.getY(), earW, outer.getHeight());
    auto rightEar = juce::Rectangle<float>(outer.getRight() - earW, outer.getY(), earW, outer.getHeight());

    auto drawRackEar = [&](juce::Rectangle<float> ear, bool flipGradient)
    {
        juce::ColourGradient earGrad(kRackEar, ear.getX(), ear.getCentreY(),
                                     kRackEarLo, ear.getRight(), ear.getCentreY(), false);
        if (flipGradient) std::swap(earGrad.point1, earGrad.point2);
        g.setGradientFill(earGrad);
        g.fillRoundedRectangle(ear, 8.0f * scale);
    };
    drawRackEar(leftEar, false);
    drawRackEar(rightEar, true);

    // ── 4. Corner screws on each rack ear ─────────────────────────────────
    auto drawScrew = [&](float cx, float cy, float r)
    {
        juce::ColourGradient sg(juce::Colour(0xFFD8CBE0), cx - r * 0.3f, cy - r * 0.3f,
                                juce::Colour(0xFF2A2230), cx + r * 0.5f, cy + r * 0.5f, true);
        g.setGradientFill(sg);
        g.fillEllipse(cx - r, cy - r, r * 2, r * 2);

        // Slot (rotated 28°)
        juce::Path slot;
        const float sl = r * 1.4f;
        const float st = r * 0.25f;
        slot.addRectangle(-sl * 0.5f, -st * 0.5f, sl, st);
        slot.applyTransform(juce::AffineTransform::rotation(0.49f).translated(cx, cy));
        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.fillPath(slot);
    };
    const float scrR = 8.0f * scale;
    const float scrInset = 16.0f * scale;
    drawScrew(leftEar.getCentreX(),  leftEar.getY()  + scrInset, scrR);
    drawScrew(leftEar.getCentreX(),  leftEar.getBottom() - scrInset, scrR);
    drawScrew(rightEar.getCentreX(), rightEar.getY()  + scrInset, scrR);
    drawScrew(rightEar.getCentreX(), rightEar.getBottom() - scrInset, scrR);

    // ── 5. Section frames with floating Pirata-One-style serif headers ────
    auto drawSectionFrame = [&](juce::Rectangle<int> rect, const juce::String& header)
    {
        if (rect.isEmpty()) return;
        const auto b = rect.toFloat();

        // 1px purple border
        g.setColour(kSectionBorder);
        g.drawRoundedRectangle(b, 6.0f * scale, 1.0f);

        // Floating header tag — serif italic bold, paints over the border line.
        const float headerSize = 22.0f * scale;
        juce::Font headerFont = rato::Fonts::pirata(headerSize);

        // Measure header text width via GlyphArrangement (JUCE 8 removed Font::getStringWidth)
        juce::GlyphArrangement ga;
        ga.addLineOfText(headerFont, header, 0.0f, 0.0f);
        const float textW = ga.getBoundingBox(0, -1, true).getWidth();

        const float tagX = b.getX() + 18.0f * scale;
        const float tagY = b.getY() - headerSize * 0.55f;
        auto tag = juce::Rectangle<float>(tagX - 8.0f * scale, tagY,
                                          textW + 16.0f * scale, headerSize);

        // Tag background masks the border
        g.setColour(kPanel);
        g.fillRect(tag);

        // Tag text
        g.setFont(headerFont);
        g.setColour(kBrandPurple);
        g.drawText(header, tag, juce::Justification::centred, false);
    };

    drawSectionFrame(sectionIOFrame,   "I/O");
    drawSectionFrame(sectionCompFrame, "COMPRESSOR");
    drawSectionFrame(sectionSatFrame,  "SATURATION");

    // ── 6. Brand title block (logo / title text area) ─────────────────────
    if (logoDrawable != nullptr && ! logoBounds.isEmpty())
    {
        logoDrawable->drawWithin(g, logoBounds.toFloat(),
                                 juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid,
                                 1.0f);

        // "compressor / distressor" subtitle next to logo
        g.setColour(kTextCream);
        g.setFont(juce::Font(juce::FontOptions(12.0f * scale,
                                               juce::Font::bold)));
        g.drawText("compressor / distressor",
                   logoBounds.translated(0, static_cast<int>(logoBounds.getHeight() * 0.55f)),
                   juce::Justification::centredRight, true);
    }
    else if (! logoBounds.isEmpty())
    {
        // Pirata One brand wordmark
        const float titleSize = juce::jmin(logoBounds.getHeight() * 0.75f, 36.0f * scale);
        juce::Font titleFont = rato::Fonts::pirata(titleSize);
        g.setFont(titleFont);

        // Soft glow underlay
        g.setColour(kBrandPurple.withAlpha(0.35f));
        for (int dy = -2; dy <= 2; ++dy)
            g.drawText("RATOSAURUS", logoBounds.translated(0, dy),
                       juce::Justification::centredLeft, true);
        // Drop-shadow
        g.setColour(juce::Colour(0xFF1A0F22));
        g.drawText("RATOSAURUS", logoBounds.translated(0, 2),
                   juce::Justification::centredLeft, true);
        // Main fill
        g.setColour(kBrandPurple);
        g.drawText("RATOSAURUS", logoBounds, juce::Justification::centredLeft, true);

        // Subtitle right of title
        g.setColour(kTextCream);
        g.setFont(rato::Fonts::oswald(12.0f * scale, true));
        g.drawText("compressor / distressor",
                   logoBounds.translated(static_cast<int>(220.0f * scale), 0),
                   juce::Justification::centredLeft, true);
    }

    // ── 7. Bottom bar — 5-column grid (POWER · UKRAINE · sig · GLORY · BYPASS)
    if (! bottomBarFrame.isEmpty())
    {
        const auto bar = bottomBarFrame;
        const int colW = bar.getWidth() / 5;
        const float ledR = 6.5f * scale;

        // ─── col 0 : POWER LED + label (left aligned) ────────────────────
        auto col0 = juce::Rectangle<int>(bar.getX(), bar.getY(), colW, bar.getHeight()).reduced(12, 4);
        const float ledX0 = col0.getX() + ledR + 4.0f;
        const float ledY0 = col0.getCentreY();
        // glow
        for (int i = 5; i >= 0; --i)
        {
            g.setColour(kMagentaHot.withAlpha(0.10f - i * 0.012f));
            g.fillEllipse(ledX0 - ledR - i, ledY0 - ledR - i, (ledR + i) * 2, (ledR + i) * 2);
        }
        g.setColour(kMagentaHot);
        g.fillEllipse(ledX0 - ledR, ledY0 - ledR, ledR * 2, ledR * 2);

        g.setColour(kTextMuted);
        g.setFont(rato::Fonts::oswald(12.0f * scale, true));
        g.drawText("POWER",
                   juce::Rectangle<int>(static_cast<int>(ledX0 + ledR + 10), col0.getY(),
                                        col0.getRight() - static_cast<int>(ledX0 + ledR + 10),
                                        col0.getHeight()),
                   juce::Justification::centredLeft, true);

        // ─── col 1 : "HANDCRAFTED IN UKRAINE" badge ──────────────────────
        auto col1 = juce::Rectangle<int>(bar.getX() + colW, bar.getY(), colW, bar.getHeight()).reduced(12, 10);
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.fillRoundedRectangle(col1.toFloat(), 4.0f * scale);
        g.setColour(kBrandPurple.withAlpha(0.30f));
        g.drawRoundedRectangle(col1.toFloat(), 4.0f * scale, 1.0f);
        g.setColour(kTextDim);
        g.setFont(rato::Fonts::oswald(10.0f * scale, true));
        g.drawFittedText("HANDCRAFTED IN\nUKRAINE", col1, juce::Justification::centred, 2);

        // ─── col 2 : signature centre ────────────────────────────────────
        auto col2 = juce::Rectangle<int>(bar.getX() + colW * 2, bar.getY(), colW, bar.getHeight());
        const float sigSize = 26.0f * scale;
        juce::Font sigFont = rato::Fonts::pirata(sigSize);
        g.setFont(sigFont);

        // Glow
        g.setColour(kBrandPurple.withAlpha(0.35f));
        for (int dy = -2; dy <= 2; ++dy)
            g.drawText("By Mykyta Shchur", col2.translated(0, dy),
                       juce::Justification::centred, true);
        // Drop shadow
        g.setColour(juce::Colour(0xFF1A0F22));
        g.drawText("By Mykyta Shchur", col2.translated(0, 2), juce::Justification::centred, true);
        // Main fill (signature purple #9A6FB0)
        g.setColour(juce::Colour(0xFF9A6FB0));
        g.drawText("By Mykyta Shchur", col2, juce::Justification::centred, true);

        // ─── col 3 : "THERAPEUTIC MUD / FOR YOUR HEALTH" badge (v4) ──────
        auto col3 = juce::Rectangle<int>(bar.getX() + colW * 3, bar.getY(), colW, bar.getHeight()).reduced(12, 10);
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.fillRoundedRectangle(col3.toFloat(), 4.0f * scale);
        g.setColour(kBrandPurple.withAlpha(0.30f));
        g.drawRoundedRectangle(col3.toFloat(), 4.0f * scale, 1.0f);
        g.setColour(kTextDim);
        g.setFont(rato::Fonts::oswald(10.0f * scale, true));
        g.drawFittedText("The healing mud\nof your sound", col3, juce::Justification::centred, 2);

        // ─── col 4 : BYPASS label + LED (right aligned, painted) ─────────
        auto col4 = juce::Rectangle<int>(bar.getX() + colW * 4, bar.getY(), colW, bar.getHeight()).reduced(12, 4);
        const bool isBypassed = bypassButton.getToggleState();

        const float ledX4 = col4.getRight() - ledR - 4.0f;
        const float ledY4 = col4.getCentreY();
        if (isBypassed)
        {
            for (int i = 5; i >= 0; --i)
            {
                g.setColour(kMagentaHot.withAlpha(0.10f - i * 0.012f));
                g.fillEllipse(ledX4 - ledR - i, ledY4 - ledR - i, (ledR + i) * 2, (ledR + i) * 2);
            }
            g.setColour(kMagentaHot);
        }
        else
        {
            g.setColour(juce::Colour(0xFF2A1830));
        }
        g.fillEllipse(ledX4 - ledR, ledY4 - ledR, ledR * 2, ledR * 2);

        g.setColour(isBypassed ? kMagentaHot : kTextMuted);
        g.setFont(rato::Fonts::oswald(12.0f * scale, true));
        g.drawText("BYPASS",
                   juce::Rectangle<int>(col4.getX(), col4.getY(),
                                        static_cast<int>(ledX4 - ledR - 10) - col4.getX(),
                                        col4.getHeight()),
                   juce::Justification::centredRight, true);
    }

    // ── 8. Saturation icon + breathing halo (Stage 10E preserved) ─────────
    if (currentSatIcon != nullptr && ! satIconBounds.isEmpty())
    {
        if (lastSatModelIndex > 0)
        {
            const double w = 2.0 * juce::MathConstants<double>::pi / 2.5;
            const float pulse = 0.55f + 0.20f * static_cast<float>(std::sin(animationTimeSec * w));

            auto haloBounds = satIconBounds.toFloat().expanded(satIconBounds.getWidth() * 0.18f);
            juce::ColourGradient halo(kMagentaHot.withAlpha(pulse),
                                      haloBounds.getCentre(),
                                      juce::Colours::transparentBlack,
                                      haloBounds.getCentre().translated(haloBounds.getWidth() * 0.5f, 0),
                                      true);
            g.setGradientFill(halo);
            g.fillEllipse(haloBounds);
        }

        currentSatIcon->drawWithin(g, satIconBounds.toFloat(),
                                   juce::RectanglePlacement::centred, 1.0f);
    }
#endif
}

void RatosaurusEditor::resized()
{
    // v4 layout. Aspect ratio locked. Use height as reference for scale.
    const float scale = static_cast<float>(getHeight()) / static_cast<float>(kWindowHeight);
    auto s = [scale](int v) { return static_cast<int>(std::round(v * scale)); };

    auto place = [s](juce::Component& c, int x, int y, int w, int h)
    {
        c.setBounds(s(x), s(y), s(w), s(h));
        c.setVisible(true);
    };

    place(inputKnob,  98, 72, 118, 118);
    place(outputKnob, 328, 72, 118, 118);
    place(inputMeterUI, 254, 88, 18, 128);
    place(outputMeterUI, 484, 88, 18, 128);
    place(vuMeter, 744, 66, 244, 164);

    if (kneeSelector != nullptr)     place(*kneeSelector, 558, 65, 88, 82);
    if (detectorSelector != nullptr) place(*detectorSelector, 558, 170, 88, 62);

    place(thresholdKnob, 101, 308, 110, 110);
    place(ratioKnob,     247, 308, 110, 110);
    place(attackKnob,    393, 308, 110, 110);
    place(releaseKnob,   539, 308, 110, 110);
    place(grMeterUI,     732, 332, 364, 48);
    if (characterSelector != nullptr) place(*characterSelector, 1184, 320, 84, 82);

    place(driveKnob,   99, 510, 104, 104);
    place(densityKnob, 259, 510, 104, 104);
    place(colorKnob,   419, 510, 104, 104);
    if (satTiles != nullptr) place(*satTiles, 580, 526, 670, 92);

    place(presetLabel, 145, 670, 55, 24);
    place(presetCombo, 202, 669, 205, 26);
    place(aButton,     422, 669, 30, 26);
    place(bButton,     456, 669, 30, 26);
    place(copyButton,  492, 669, 54, 26);
    place(oversamplingCombo, 1158, 669, 48, 26);
    place(bypassButton, 1210, 666, 112, 32);

    logoBounds = { s(1115), s(70), s(100), s(100) };
    signatureBounds = {};
    satIconBounds = {};

    for (auto* label : { &inputLabel, &outputLabel, &thresholdLabel, &ratioLabel,
                         &attackLabel, &releaseLabel, &driveLabel, &densityLabel,
                         &colorLabel, &grLabel, &sectionIO, &sectionComp,
                         &sectionChar, &sectionSat })
        label->setVisible(false);

    return;

#if 0

    // Inset for content within chassis (skip rack ears + chassis padding).
    const int innerX = s(kChassisPadding + kRackEarWidth + 10);
    const int innerY = s(kChassisPadding + 8);
    auto area = juce::Rectangle<int>(innerX, innerY,
                                     getWidth()  - innerX * 2,
                                     getHeight() - innerY * 2);

    // ── Title area — logo / "RATOSAURUS" title block ──────────────────────
    {
        auto titleArea = area.removeFromTop(s(kTitleHeight));
        logoBounds = titleArea.removeFromLeft(s(kLogoMaxWidth)).reduced(s(8), s(4));
    }

    // signature legacy bounds — drawn via bottom bar now
    signatureBounds = juce::Rectangle<int>();

    // ── Bottom bar (reserved from bottom BEFORE sections) ─────────────────
    bottomBarFrame = area.removeFromBottom(s(kBottomBarH));
    {
        // Place bypassButton (transparent) over the BYPASS LED column for click handling
        const int colW = bottomBarFrame.getWidth() / 5;
        auto col4 = juce::Rectangle<int>(bottomBarFrame.getX() + colW * 4,
                                         bottomBarFrame.getY(),
                                         colW,
                                         bottomBarFrame.getHeight()).reduced(s(8), s(6));
        bypassButton.setBounds(col4);
    }
    area.removeFromBottom(s(8));

    // ── Top bar: presets + A/B/Copy (bypass now in bottom bar) ────────────
    {
        auto bar = area.removeFromTop(s(kTopBarHeight)).reduced(s(4), s(4));

        presetLabel.setBounds(bar.removeFromLeft(s(56)));
        bar.removeFromLeft(s(4));
        presetCombo.setBounds(bar.removeFromLeft(s(220)));

        copyButton.setBounds(bar.removeFromRight(s(60)));
        bar.removeFromRight(s(6));
        bButton.setBounds(bar.removeFromRight(s(30)));
        bar.removeFromRight(s(3));
        aButton.setBounds(bar.removeFromRight(s(30)));
    }
    area.removeFromTop(s(10));

    auto layKnob = [&](juce::Rectangle<int>& row, juce::Slider& knob, juce::Label& label)
    {
        auto col = row.removeFromLeft(s(kKnobWidth));
        label.setBounds(col.removeFromTop(s(kLabelHeight)));
        knob.setBounds(col.reduced(s(2), 0));
    };

    auto layKnobWithMeter = [&](juce::Rectangle<int>& row, juce::Slider& knob,
                                juce::Label& label, rato::RatoMeter& meter)
    {
        const int slotW = s(kKnobWidth + kMeterGap + kMeterWidth);
        auto slot = row.removeFromLeft(slotW);

        auto knobCol = slot.removeFromLeft(s(kKnobWidth));
        label.setBounds(knobCol.removeFromTop(s(kLabelHeight)));
        knob.setBounds(knobCol.reduced(s(2), 0));

        slot.removeFromLeft(s(kMeterGap));
        slot.removeFromTop(s(kLabelHeight));
        meter.setBounds(slot.reduced(0, s(4)));
    };

    auto layCombo = [&](juce::Rectangle<int>& row, juce::ComboBox& combo, juce::Label& label)
    {
        auto col = row.removeFromLeft(s(kComboWidth));
        label.setBounds(col.removeFromTop(s(kLabelHeight)));
        auto comboArea = col.removeFromTop(s(kComboHeight + 4)).reduced(s(6), s(2));
        combo.setBounds(comboArea);
    };

    // v4 R1: place a RatoLedSelector occupying the full content-row height.
    auto layLedSelector = [&](juce::Rectangle<int>& row, rato::RatoLedSelector& sel, int widthPx)
    {
        auto col = row.removeFromLeft(s(widthPx));
        sel.setBounds(col);
    };

    // v4: divide remaining area into 3 equal sections with gaps. Each section
    // is its own framed box that paint() draws with a floating serif header.
    const int sectionGap = s(kSectionGap);
    const int totalSectionH = area.getHeight() - sectionGap * 2;
    const int oneSectionH = totalSectionH / 3;

    auto ioSection   = area.removeFromTop(oneSectionH);
    area.removeFromTop(sectionGap);
    auto compSection = area.removeFromTop(oneSectionH);
    area.removeFromTop(sectionGap);
    auto satSection  = area.removeFromTop(oneSectionH);

    // Store frames for paint() to draw borders + floating headers
    sectionIOFrame   = ioSection;
    sectionCompFrame = compSection;
    sectionSatFrame  = satSection;

    // Inset for content within each section (leave room for floating header at top)
    auto contentArea = [&](juce::Rectangle<int> section)
    {
        return section.reduced(s(14), s(10)).withTrimmedTop(s(8));
    };

    // ── I/O content row ─────────────────────────────────────────────────
    {
        auto row = contentArea(ioSection);
        layKnobWithMeter(row, inputKnob,  inputLabel,  inputMeterUI);
        row.removeFromLeft(s(10));
        layKnobWithMeter(row, outputKnob, outputLabel, outputMeterUI);
        row.removeFromLeft(s(20));
        if (kneeSelector != nullptr)     layLedSelector(row, *kneeSelector, 90);
        row.removeFromLeft(s(14));
        if (detectorSelector != nullptr) layLedSelector(row, *detectorSelector, 90);
    }

    // ── Compressor content row ──────────────────────────────────────────
    {
        auto row = contentArea(compSection);
        layKnob (row, thresholdKnob, thresholdLabel);
        layKnob (row, ratioKnob,     ratioLabel);
        layKnob (row, attackKnob,    attackLabel);
        layKnob (row, releaseKnob,   releaseLabel);
        row.removeFromLeft(s(14));
        if (characterSelector != nullptr) layLedSelector(row, *characterSelector, 100);

        // GR meter on the far right
        auto grSlot = row.removeFromRight(s(kGrMeterWidth + 10));
        grSlot.removeFromRight(s(10));
        grLabel.setBounds(grSlot.removeFromTop(s(kLabelHeight)));
        grMeterUI.setBounds(grSlot.reduced(0, s(4)));
    }

    // ── Saturation content row ──────────────────────────────────────────
    {
        auto row = contentArea(satSection);
        layKnob (row, driveKnob,        driveLabel);
        layKnob (row, densityKnob,      densityLabel);
        layKnob (row, colorKnob,        colorLabel);
        row.removeFromLeft(s(20));

        // SATURATION MODEL tiles header + tile row
        if (satTiles != nullptr)
        {
            row.removeFromTop(s(kLabelHeight));        // leave room for header text
            const int tilesW = juce::jmin(row.getWidth(), s(540));
            satTiles->setBounds(row.removeFromLeft(tilesW).reduced(0, s(2)));
        }

        satIconBounds = juce::Rectangle<int>();        // old single icon retired
    }

    // Hide JUCE-Label section headers — paint() handles them as serif tags
    sectionIO.setVisible(false);
    sectionComp.setVisible(false);
    sectionChar.setVisible(false);
    sectionSat.setVisible(false);
#endif
}

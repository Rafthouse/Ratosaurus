#include "RatoSatTiles.h"
#include "AssetCache.h"
#include "RatoFonts.h"

namespace rato
{

namespace
{
    const juce::Colour kHotMagenta   { 0xFFFF8DF5 };
    const juce::Colour kPurpleBright  { 0xFFB06FD0 };
    const juce::Colour kCream         { 0xFFC9B8A0 };
    const juce::Colour kDim           { 0xFF8C7A96 };
    const juce::Colour kLedOff        { 0xFF2A1830 };
    const juce::Colour kIconActive    { 0xFFF0E6D2 };
    const juce::Colour kIconRatActive { 0xFFFFB3F7 };
    const juce::Colour kIconDim       { 0xFF8A7A96 };

    constexpr int kTileGap = 10;

    void keepOnlySlice(juce::Drawable* drawable, juce::Rectangle<float> slice)
    {
        auto* composite = dynamic_cast<juce::DrawableComposite*>(drawable);
        if (composite == nullptr)
            return;

        for (int childIndex = composite->getNumChildComponents() - 1;
             childIndex >= 0; --childIndex)
        {
            auto* child = composite->getChildComponent(childIndex);
            auto* childDrawable = dynamic_cast<juce::Drawable*>(child);
            if (childDrawable == nullptr)
                continue;

            auto childBounds = childDrawable->getDrawableBounds();
            if (childDrawable->isTransformed())
                childBounds = childBounds.transformedBy(childDrawable->getTransform());

            if (! slice.contains(childBounds.getCentre()))
            {
                composite->removeChildComponent(childDrawable);
                delete childDrawable;
            }
        }

        composite->resetContentAreaAndBoundingBoxToFitChildren();
    }
}

RatoSatTiles::RatoSatTiles(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& paramId)
    : parameter(apvts.getParameter(paramId)),
      attachment(*parameter, [this](float v) { onParameterChanged(v); })
{
    jassert(parameter != nullptr);

    using Asset = AssetCache::Asset;
    const std::array<Asset, kNumTiles> keys {
        Asset::IconOff, Asset::IconTape, Asset::IconTube, Asset::IconFet, Asset::IconRat
    };
    auto& cache = AssetCache::getInstance();
    for (int i = 0; i < kNumTiles; ++i)
        icons[static_cast<size_t>(i)] = cache.createDrawable(keys[static_cast<size_t>(i)]);

    static const std::array<juce::Rectangle<float>, kNumTiles> slices {
        juce::Rectangle<float>{ -10.0f, -5.0f, 300.0f, 310.0f },
        juce::Rectangle<float>{ 290.0f, -5.0f, 300.0f, 310.0f },
        juce::Rectangle<float>{ 585.0f, -5.0f, 300.0f, 310.0f },
        juce::Rectangle<float>{ 875.0f, -5.0f, 300.0f, 310.0f },
        juce::Rectangle<float>{ 1160.0f, -5.0f, 310.0f, 310.0f }
    };
    for (int i = 0; i < kNumTiles; ++i)
        keepOnlySlice(icons[static_cast<size_t>(i)].get(), slices[static_cast<size_t>(i)]);

    attachment.sendInitialUpdate();
}

void RatoSatTiles::onParameterChanged(float denormalised)
{
    const int idx = juce::jlimit(0, kNumTiles - 1, static_cast<int>(std::round(denormalised)));
    if (idx != currentIndex)
    {
        currentIndex = idx;
        repaint();
    }
}

juce::Rectangle<float> RatoSatTiles::tileBounds(int i) const
{
    const float totalGap = kTileGap * (kNumTiles - 1);
    const float tileW = (getWidth() - totalGap) / static_cast<float>(kNumTiles);
    const float x = i * (tileW + kTileGap);
    return { x, 0.0f, tileW, static_cast<float>(getHeight()) };
}

int RatoSatTiles::tileAtX(int x) const
{
    for (int i = 0; i < kNumTiles; ++i)
        if (tileBounds(i).contains(static_cast<float>(x), 1.0f))
            return i;
    return -1;
}

void RatoSatTiles::paint(juce::Graphics& g)
{
    for (int i = 0; i < kNumTiles; ++i)
    {
        const auto tb = tileBounds(i).reduced(4.0f, 2.0f);
        const bool active = (i == currentIndex);
        const bool isRat  = (i == kNumTiles - 1);

        // ── Tile background ──────────────────────────────────────────────
        if (active && isRat)
        {
            juce::ColourGradient bg(juce::Colour(0xFF5A1F62), tb.getCentreX(), tb.getY() + tb.getHeight() * 0.3f,
                                    juce::Colour(0xFF1A0F22), tb.getCentreX(), tb.getBottom(), true);
            g.setGradientFill(bg);
        }
        else if (active)
        {
            juce::ColourGradient bg(juce::Colour(0xFF3A2A44), tb.getCentreX(), tb.getY(),
                                    juce::Colour(0xFF1C1226), tb.getCentreX(), tb.getBottom(), false);
            g.setGradientFill(bg);
        }
        else
        {
            juce::ColourGradient bg(juce::Colour(0xFF221826), tb.getCentreX(), tb.getY(),
                                    juce::Colour(0xFF140D1A), tb.getCentreX(), tb.getBottom(), false);
            g.setGradientFill(bg);
        }
        g.fillRoundedRectangle(tb, 6.0f);

        // ── Active glow (RAT only) ───────────────────────────────────────
        if (active && isRat)
        {
            for (int gi = 4; gi >= 0; --gi)
            {
                g.setColour(kHotMagenta.withAlpha(0.10f - gi * 0.018f));
                g.drawRoundedRectangle(tb.expanded(static_cast<float>(gi)), 6.0f, 1.5f);
            }
        }

        // ── Border ───────────────────────────────────────────────────────
        if (active)
            g.setColour(isRat ? kHotMagenta.withAlpha(0.7f) : kPurpleBright.withAlpha(0.6f));
        else
            g.setColour(kPurpleBright.withAlpha(0.22f));
        g.drawRoundedRectangle(tb, 6.0f, 1.0f);

        // ── LED dot (top centre) ─────────────────────────────────────────
        const float ledR = 3.5f;
        const float ledX = tb.getCentreX();
        const float ledY = tb.getY() + 10.0f;
        if (active)
        {
            const auto ledCol = isRat ? kHotMagenta : kCream;
            for (int gi = 4; gi >= 0; --gi)
            {
                g.setColour(ledCol.withAlpha(0.20f - gi * 0.035f));
                const float er = ledR + 1.0f + gi;
                g.fillEllipse(ledX - er, ledY - er, er * 2.0f, er * 2.0f);
            }
            g.setColour(ledCol);
        }
        else
        {
            g.setColour(kLedOff);
        }
        g.fillEllipse(ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);

        // ── Icon (middle) — slice the shared sprite-sheet SVG ───────────
        // All control SVGs are the same sheet (5 icons in one 1450-wide canvas);
        // each icon is a 290-unit-wide slice. We map the slice into the icon
        // area via transform + clip so only this tile's glyph shows.
        if (auto* icon = icons[static_cast<size_t>(i)].get())
        {
            const float iconSz = juce::jmin(tb.getWidth(), tb.getHeight()) * 0.40f;
            auto iconArea = juce::Rectangle<float>(iconSz, iconSz)
                                .withCentre({ tb.getCentreX(), tb.getCentreY() + 2.0f });
            icon->drawWithin(g, iconArea, juce::RectanglePlacement::centred,
                             active ? 1.0f : 0.48f);
        }

        // ── Label (bottom) ───────────────────────────────────────────────
        g.setColour(active ? (isRat ? kHotMagenta : kCream) : kDim);
        g.setFont(Fonts::oswald(13.0f, true));
        auto labelArea = juce::Rectangle<float>(tb.getX(), tb.getBottom() - 18.0f, tb.getWidth(), 14.0f);
        g.drawText(labels[static_cast<size_t>(i)], labelArea.toNearestInt(),
                   juce::Justification::centred, false);
    }
}

void RatoSatTiles::mouseDown(const juce::MouseEvent& e)
{
    const int idx = tileAtX(e.x);
    if (idx >= 0 && idx != currentIndex && parameter != nullptr)
        attachment.setValueAsCompleteGesture(static_cast<float>(idx));
}

} // namespace rato

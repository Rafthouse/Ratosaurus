#include "RatosaurusLookAndFeel.h"
#include "RatoKnob.h"
#include "RatoFonts.h"
#include "AssetCache.h"
#include <cmath>

namespace rato
{

namespace c = ::rato::theme::colour;
namespace f = ::rato::theme::font;

namespace
{
    // --------------------------------------------------------------------
    //  Helpers used by every knob style.
    // --------------------------------------------------------------------
    inline void dropShadowEllipse(juce::Graphics& g, juce::Point<float> centre,
                                  float r, float radius, float dy, float alpha)
    {
        juce::DropShadow shadow(juce::Colours::black.withAlpha(alpha),
                                static_cast<int>(radius),
                                juce::Point<int>(0, static_cast<int>(dy)));
        juce::Path p;
        p.addEllipse(centre.x - r, centre.y - r, r * 2.0f, r * 2.0f);
        shadow.drawForPath(g, p);
    }

    inline void fillEllipseCentred(juce::Graphics& g, juce::Point<float> centre, float r)
    {
        g.fillEllipse(centre.x - r, centre.y - r, r * 2.0f, r * 2.0f);
    }

    inline void hoverHalo(juce::Graphics& g, juce::Point<float> centre, float r, bool hovered)
    {
        if (! hovered) return;
        const float haloR = r * 1.10f;
        g.setColour(juce::Colour(c::knobHaloHover).withAlpha(0.55f));
        g.drawEllipse(centre.x - haloR, centre.y - haloR, haloR * 2.0f, haloR * 2.0f, 1.2f);
    }

    // Draws a thin pointer line rotated by `angle` (radians, 0 = up, +cw),
    // from radius `innerR` to `outerR` with given colour and width.
    inline void drawPointerLine(juce::Graphics& g, juce::Point<float> centre,
                                float angle, float innerR, float outerR,
                                juce::Colour col, float thickness)
    {
        const float c0 = std::cos(angle - juce::MathConstants<float>::halfPi);
        const float s0 = std::sin(angle - juce::MathConstants<float>::halfPi);
        juce::Line<float> ln(centre.translated(c0 * innerR, s0 * innerR),
                             centre.translated(c0 * outerR, s0 * outerR));
        g.setColour(col);
        g.drawLine(ln, thickness);
    }

    // ====================================================================
    //  #1  BAKELITE POINTER  (Teletronix LA-2A / Fairchild 660)
    // ====================================================================
    void paintBakelitePointer(juce::Graphics& g, juce::Rectangle<float> bounds,
                              float angle, bool hovered)
    {
        const auto centre = bounds.getCentre();
        const float r = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

        dropShadowEllipse(g, centre, r, r * 0.16f, r * 0.10f, 0.70f);

        // Body — radial dark grey to near-black
        {
            juce::ColourGradient body(juce::Colour(0xFF4A4046),
                                      centre.x - r * 0.16f, centre.y - r * 0.40f,
                                      juce::Colour(0xFF0A0809),
                                      centre.x + r * 0.50f, centre.y + r * 0.65f, true);
            body.addColour(0.38, juce::Colour(0xFF2A2428));
            body.addColour(0.78, juce::Colour(0xFF161316));
            g.setGradientFill(body);
            fillEllipseCentred(g, centre, r);
        }

        // Chamfer inner ring (inset ~8 %)
        const float chamferR = r * 0.92f;
        {
            juce::ColourGradient ch(juce::Colour(0xFF3A3338),
                                    centre.x - chamferR * 0.12f, centre.y - chamferR * 0.36f,
                                    juce::Colour(0xFF1A1518),
                                    centre.x + chamferR * 0.40f, centre.y + chamferR * 0.55f, true);
            g.setGradientFill(ch);
            fillEllipseCentred(g, centre, chamferR);
        }

        // Raised cream pointer ridge, rotated to current angle
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(angle, centre.x, centre.y));

            const float pw = std::max(2.0f, r * 0.045f);
            const float ph = r * 0.40f;
            const float top = centre.y - r * 0.88f;
            juce::Rectangle<float> rect(centre.x - pw * 0.5f, top, pw, ph);

            juce::ColourGradient pg(juce::Colour(0xFFFFF7E6), centre.x, top,
                                    juce::Colour(0xFFCDBFD6), centre.x, top + ph, false);
            g.setGradientFill(pg);
            g.fillRoundedRectangle(rect, pw * 0.5f);
        }

        // Brass centre cap (~52 % diameter)
        const float capR = r * 0.27f;
        {
            juce::ColourGradient cap(juce::Colour(0xFFE8D49A),
                                     centre.x - capR * 0.20f, centre.y - capR * 0.40f,
                                     juce::Colour(0xFF5A4A2C),
                                     centre.x + capR * 0.40f, centre.y + capR * 0.60f, true);
            cap.addColour(0.55, juce::Colour(0xFFA8915C));
            g.setGradientFill(cap);
            fillEllipseCentred(g, centre, capR);
        }

        // Tiny dark screw dot
        const float dotR = capR * 0.18f;
        g.setColour(juce::Colour(0xFF2E2412));
        fillEllipseCentred(g, centre, dotR);

        hoverHalo(g, centre, r, hovered);
    }

    // ====================================================================
    //  #2  BRUSHED METAL  (10-step spec from Knob Build.dc.html)
    //
    //  Reference radius 256 → r below maps the rest:
    //    outer disc  = r           (1.000)
    //    bevel ring  = r * 0.980   (196/200)
    //    knurl tips  = r * 0.950   (190/200)  outer
    //                  r * 0.850   (170/200)  inner
    //    inner cup   = r * 0.664   (170/256)
    //    centre cup  = r * 0.586   (150/256)
    //    pointer top = r * 0.727   (186/256)  outward tip
    //    pointer bot = r * 0.367   (94/256)   inward base
    //    pointer w   = r * 0.0469  (12/256)
    //
    //  Colours kept exactly per spec — metal gradient + bevel + center.
    // ====================================================================
    void paintBrushedAlu(juce::Graphics& g, juce::Rectangle<float> bounds,
                         float angle, bool hovered)
    {
        const auto centre = bounds.getCentre();
        const float r = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

        // ── 1. Seating drop shadow under knob ─────────────────────────────
        dropShadowEllipse(g, centre, r * 0.99f, r * 0.18f, r * 0.16f, 0.55f);

        // ── 2. Outer brushed metal disc ───────────────────────────────────
        //     Diagonal 135° linear gradient: F4 → DA → A9 → E0 → FA
        {
            const auto p0 = juce::Point<float>(centre.x - r * 0.7f, centre.y - r * 0.7f);
            const auto p1 = juce::Point<float>(centre.x + r * 0.7f, centre.y + r * 0.7f);
            juce::ColourGradient metal(juce::Colour(0xFFF4F4F4), p0,
                                       juce::Colour(0xFFFAFAFA), p1, false);
            metal.addColour(0.25, juce::Colour(0xFFDADADA));
            metal.addColour(0.50, juce::Colour(0xFFA9A9A9));
            metal.addColour(0.75, juce::Colour(0xFFE0E0E0));
            g.setGradientFill(metal);
            fillEllipseCentred(g, centre, r);
        }
        // Top-left metal sheen (radial 42%/30%, white 55% → 0)
        {
            juce::ColourGradient sheen(juce::Colours::white.withAlpha(0.55f),
                                       centre.x - r * 0.16f, centre.y - r * 0.40f,
                                       juce::Colours::transparentWhite,
                                       centre.x + r * 0.55f, centre.y + r * 0.20f, true);
            g.setGradientFill(sheen);
            fillEllipseCentred(g, centre, r);
        }

        // ── 3. Bevel ring (8/256 ≈ 3% stroke) ─────────────────────────────
        //     Vertical light→dark gradient gives the rim thickness
        {
            const float bevelR = r * 0.980f;
            juce::ColourGradient bevel(juce::Colours::white,
                                       centre.x, centre.y - bevelR,
                                       juce::Colour(0xFF7E7E7E),
                                       centre.x, centre.y + bevelR, false);
            g.setGradientFill(bevel);
            const float strokeW = std::max(2.0f, r * 0.040f);
            g.drawEllipse(centre.x - bevelR, centre.y - bevelR,
                          bevelR * 2.0f, bevelR * 2.0f, strokeW);
        }

        // ── 4 (deferred). Knurling — 72 ticks every 5°, rotates with knob ─
        //     Lines from r=170 to r=190 (relative scale).
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(angle, centre.x, centre.y));
            const float inner = r * 0.850f;
            const float outer = r * 0.950f;
            const float tickW = std::max(1.0f, r * 0.010f);
            g.setColour(juce::Colour(0xFF6B6B6B));
            for (int i = 0; i < 72; ++i)
            {
                const float tickAng = (i * 5.0f) * juce::MathConstants<float>::pi / 180.0f
                                      - juce::MathConstants<float>::halfPi;
                const float c0 = std::cos(tickAng), s0 = std::sin(tickAng);
                juce::Line<float> tk(centre.translated(c0 * inner, s0 * inner),
                                     centre.translated(c0 * outer, s0 * outer));
                g.drawLine(tk, tickW);
            }
        }

        // ── 5. Inner cup (r=170/256 ≈ 0.664) ──────────────────────────────
        const float cupR = r * 0.664f;
        g.setColour(juce::Colour(0xFFBEBEBE));
        fillEllipseCentred(g, centre, cupR);
        g.setColour(juce::Colour(0xFF8C8C8C));
        g.drawEllipse(centre.x - cupR, centre.y - cupR, cupR * 2.0f, cupR * 2.0f,
                      std::max(0.8f, r * 0.008f));

        // ── 6. Centre recess with vertical gradient (r=150/256 ≈ 0.586) ───
        const float recR = r * 0.586f;
        {
            juce::ColourGradient cen(juce::Colour(0xFF707070),
                                     centre.x, centre.y - recR,
                                     juce::Colour(0xFFC0C0C0),
                                     centre.x, centre.y + recR, false);
            g.setGradientFill(cen);
            fillEllipseCentred(g, centre, recR);
        }
        // Recess inner edge shadow
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.drawEllipse(centre.x - recR, centre.y - recR, recR * 2.0f, recR * 2.0f,
                      std::max(1.5f, r * 0.014f));

        // ── 7. Top white highlight ellipse (cy=150, rx=130, ry=42 @ 22%) ──
        //     Approximated by partial-alpha ellipse without blur filter
        //     (JUCE has no native blur; layered alpha gives a soft band).
        {
            const float ehx = centre.x;
            const float ehy = centre.y - r * 0.414f;     // 256 - 150 = 106 / 256 = 0.414
            const float erx = r * 0.508f;                // 130 / 256
            const float ery = r * 0.164f;                // 42  / 256
            for (int i = 4; i >= 0; --i)
            {
                const float exp = 1.0f + 0.10f * i;
                g.setColour(juce::Colours::white.withAlpha(0.22f / (i + 1.0f)));
                g.fillEllipse(ehx - erx * exp, ehy - ery * exp,
                              erx * 2.0f * exp, ery * 2.0f * exp);
            }
        }

        // ── 8. Bottom inner shadow ellipse (cy=372, rx=120, ry=40 @ 14%) ──
        {
            const float esx = centre.x;
            const float esy = centre.y + r * 0.453f;     // 372 - 256 = 116 / 256 ≈ 0.453
            const float erx = r * 0.469f;
            const float ery = r * 0.156f;
            for (int i = 4; i >= 0; --i)
            {
                const float exp = 1.0f + 0.10f * i;
                g.setColour(juce::Colours::black.withAlpha(0.14f / (i + 1.0f)));
                g.fillEllipse(esx - erx * exp, esy - ery * exp,
                              erx * 2.0f * exp, ery * 2.0f * exp);
            }
        }

        // ── 9. Pointer (12×92, rotates with knob) ─────────────────────────
        //     Dark #222 outer + lighter #3c3c3c center stripe.
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(angle, centre.x, centre.y));

            const float pw  = std::max(3.0f, r * 0.0469f);
            const float pH  = r * 0.359f;                // 92/256
            const float pY0 = centre.y - r * 0.727f;     // top edge of pointer
            const float corner = std::max(1.2f, pw * 0.4f);

            g.setColour(juce::Colour(0xFF222222));
            g.fillRoundedRectangle(centre.x - pw * 0.5f, pY0, pw, pH, corner);

            // Inner subtle stripe
            const float ipw = std::max(1.0f, pw * 0.25f);
            g.setColour(juce::Colour(0xFF3C3C3C));
            g.fillRoundedRectangle(centre.x - ipw * 0.5f, pY0 + pw * 0.16f,
                                   ipw, pH - pw * 0.32f, ipw * 0.4f);
        }

        // ── 10. Hover magenta glow ring ───────────────────────────────────
        if (hovered)
        {
            // Outer soft glow — multi-stroke alpha falloff (faux blur).
            for (int i = 0; i < 6; ++i)
            {
                const float gr = r * (1.040f + i * 0.012f);
                g.setColour(juce::Colour(0xFFCF57E0).withAlpha(0.42f - i * 0.06f));
                g.drawEllipse(centre.x - gr, centre.y - gr, gr * 2.0f, gr * 2.0f,
                              std::max(1.0f, r * 0.012f));
            }
            // Sharp inner ring (hot magenta)
            const float ir = r * 1.030f;
            g.setColour(juce::Colour(0xFFE87BF5).withAlpha(0.90f));
            g.drawEllipse(centre.x - ir, centre.y - ir, ir * 2.0f, ir * 2.0f,
                          std::max(0.8f, r * 0.008f));
        }
    }

    // ====================================================================
    //  #3  SOVIET FLUTED BAKELITE  (eastern-european lab apparatus)
    // ====================================================================
    void paintSovietFluted(juce::Graphics& g, juce::Rectangle<float> bounds,
                           float angle, bool hovered)
    {
        const auto centre = bounds.getCentre();
        const float r = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

        dropShadowEllipse(g, centre, r, r * 0.16f, r * 0.10f, 0.70f);

        // 36 alternating ridge/groove sectors
        const int flutes = 36;
        const float dAng = juce::MathConstants<float>::twoPi / flutes;
        for (int i = 0; i < flutes; ++i)
        {
            const float a0 = -juce::MathConstants<float>::halfPi + i * dAng;
            const float aMid = a0 + dAng * 0.5f;

            juce::Path wedge;
            wedge.startNewSubPath(centre);
            wedge.addPieSegment(centre.x - r, centre.y - r, r * 2.0f, r * 2.0f, a0, aMid, 0.0f);
            wedge.closeSubPath();
            g.setColour(juce::Colour(c::knobOuter));
            g.fillPath(wedge);

            juce::Path groove;
            groove.startNewSubPath(centre);
            groove.addPieSegment(centre.x - r, centre.y - r, r * 2.0f, r * 2.0f,
                                 aMid, a0 + dAng, 0.0f);
            groove.closeSubPath();
            g.setColour(juce::Colour(c::knobBase));
            g.fillPath(groove);
        }

        // Ambient purple top-light
        {
            juce::ColourGradient lit(juce::Colour(c::amberLow).withAlpha(0.30f),
                                     centre.x - r * 0.3f, centre.y - r * 0.5f,
                                     juce::Colours::transparentBlack,
                                     centre.x, centre.y + r * 0.4f, true);
            g.setGradientFill(lit);
            fillEllipseCentred(g, centre, r);
        }

        // Olive-black dome
        const float domeR = r * 0.72f;
        {
            juce::ColourGradient dome(juce::Colour(c::knobCapHigh),
                                      centre.x - domeR * 0.3f, centre.y - domeR * 0.4f,
                                      juce::Colour(c::knobCapDeep),
                                      centre.x + domeR * 0.4f, centre.y + domeR * 0.6f, true);
            dome.addColour(0.55, juce::Colour(c::knobInner));
            g.setGradientFill(dome);
            fillEllipseCentred(g, centre, domeR);
        }
        g.setColour(juce::Colour(c::wellBlack).withAlpha(0.7f));
        g.drawEllipse(centre.x - domeR, centre.y - domeR, domeR * 2.0f, domeR * 2.0f, 0.8f);

        // Engraved pointer with shadow
        const float pIn  = domeR * 0.28f;
        const float pOut = r * 0.92f;
        const auto pointerCol = hovered ? juce::Colour(c::knobPointerHot)
                                        : juce::Colour(c::knobPointerHigh);
        const float thk = std::max(1.5f, r * 0.045f);
        drawPointerLine(g, centre, angle, pIn, pOut,
                        juce::Colour(c::wellBlack).withAlpha(0.7f), thk + 1.0f);
        drawPointerLine(g, centre, angle, pIn, pOut, pointerCol, thk);

        // Centre screw
        const float screwR = std::max(2.0f, r * 0.08f);
        g.setColour(juce::Colour(c::knobCapDeep));
        fillEllipseCentred(g, centre, screwR);
        g.setColour(juce::Colour(c::brassPatina).withAlpha(0.5f));
        g.drawEllipse(centre.x - screwR, centre.y - screwR, screwR * 2.0f, screwR * 2.0f, 0.4f);

        hoverHalo(g, centre, r, hovered);
    }

    // ====================================================================
    //  #4  OXBLOOD + METAL CAP  (Pultec EQP-1A)
    // ====================================================================
    void paintOxblood(juce::Graphics& g, juce::Rectangle<float> bounds,
                      float angle, bool hovered)
    {
        const auto centre = bounds.getCentre();
        const float r = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

        dropShadowEllipse(g, centre, r, r * 0.16f, r * 0.10f, 0.65f);

        // Glossy oxblood body
        {
            juce::ColourGradient body(juce::Colour(0xFF7A2F38),
                                      centre.x - r * 0.20f, centre.y - r * 0.48f,
                                      juce::Colour(0xFF170609),
                                      centre.x + r * 0.45f, centre.y + r * 0.70f, true);
            body.addColour(0.40, juce::Colour(0xFF511C26));
            body.addColour(0.82, juce::Colour(0xFF2C0D14));
            g.setGradientFill(body);
            fillEllipseCentred(g, centre, r);
        }

        // Glossy specular highlight on upper-left
        {
            juce::ColourGradient hl(juce::Colour(0xFFFFD2D2).withAlpha(0.50f),
                                    centre.x - r * 0.26f, centre.y - r * 0.40f,
                                    juce::Colours::transparentWhite,
                                    centre.x + r * 0.10f, centre.y - r * 0.05f, true);
            g.setGradientFill(hl);
            const float hlR = r * 0.55f;
            g.fillEllipse(centre.x - r * 0.28f, centre.y - r * 0.42f, hlR * 1.4f, hlR);
        }

        // Beveled brass dome cap (~43 % diameter)
        const float capR = r * 0.43f;
        {
            juce::ColourGradient cap(juce::Colour(0xFFEFE6D0),
                                     centre.x - capR * 0.20f, centre.y - capR * 0.40f,
                                     juce::Colour(0xFF6A5A38),
                                     centre.x + capR * 0.45f, centre.y + capR * 0.60f, true);
            cap.addColour(0.45, juce::Colour(0xFFB8A878));
            g.setGradientFill(cap);
            fillEllipseCentred(g, centre, capR);
        }

        // Pointer dot on the rim (rotates)
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(angle, centre.x, centre.y));

            const float dotR = std::max(2.0f, r * 0.075f);
            const float dotY = centre.y - r * 0.85f;
            juce::ColourGradient dotG(juce::Colour(0xFFFFF4E6), centre.x, dotY - dotR,
                                      juce::Colour(0xFFCDBFA0), centre.x, dotY + dotR, false);
            g.setGradientFill(dotG);
            g.fillEllipse(centre.x - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
            if (hovered)
            {
                g.setColour(juce::Colour(c::knobPointerHot).withAlpha(0.7f));
                const float glowR = dotR * 2.2f;
                g.fillEllipse(centre.x - glowR, dotY - glowR, glowR * 2.0f, glowR * 2.0f);
            }
        }

        hoverHalo(g, centre, r, hovered);
    }

    // ====================================================================
    //  #5  LAB INSTRUMENT DIAL  (military meter 1950-70)
    // ====================================================================
    void paintLabDial(juce::Graphics& g, juce::Rectangle<float> bounds,
                      float angle, bool hovered)
    {
        const auto centre = bounds.getCentre();
        const float r = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

        dropShadowEllipse(g, centre, r, r * 0.12f, r * 0.08f, 0.6f);

        // Dark skirt with numbered ticks
        {
            juce::ColourGradient skirt(juce::Colour(0xFF2A2228),
                                       centre.x, centre.y - r * 0.10f,
                                       juce::Colour(0xFF15111A),
                                       centre.x, centre.y + r * 0.80f, true);
            g.setGradientFill(skirt);
            fillEllipseCentred(g, centre, r);
        }
        // Numbered ticks every 27° (13 marks over 270° + extras)
        {
            const int ticks = 13;
            g.setColour(juce::Colour(0xFF6A6058));
            const float startAng = -juce::MathConstants<float>::pi * 0.75f;
            const float sweep    =  juce::MathConstants<float>::pi * 1.50f;
            for (int i = 0; i < ticks; ++i)
            {
                const float t = (ticks <= 1) ? 0.5f : float(i) / float(ticks - 1);
                const float a = startAng + sweep * t - juce::MathConstants<float>::halfPi;
                const float c0 = std::cos(a), s0 = std::sin(a);
                juce::Line<float> tk(centre.translated(c0 * r * 0.82f, s0 * r * 0.82f),
                                     centre.translated(c0 * r * 0.97f, s0 * r * 0.97f));
                g.drawLine(tk, std::max(0.8f, r * 0.018f));
            }
        }

        // Knurled metal collar (inset ~26 %), simulated with 60 alternating wedges
        const float collarR = r * 0.74f;
        {
            const int notches = 60;
            const float dAng = juce::MathConstants<float>::twoPi / notches;
            for (int i = 0; i < notches; ++i)
            {
                const float a0 = -juce::MathConstants<float>::halfPi + i * dAng;
                const float a1 = a0 + dAng;
                juce::Path w;
                w.startNewSubPath(centre);
                w.addPieSegment(centre.x - collarR, centre.y - collarR,
                                collarR * 2.0f, collarR * 2.0f, a0, a1, 0.0f);
                w.closeSubPath();
                g.setColour((i & 1) ? juce::Colour(0xFF4A444A) : juce::Colour(0xFF8A8288));
                g.fillPath(w);
            }
        }
        // Collar specular highlight
        {
            juce::ColourGradient hl(juce::Colours::white.withAlpha(0.40f),
                                    centre.x - collarR * 0.20f, centre.y - collarR * 0.45f,
                                    juce::Colours::transparentWhite,
                                    centre.x + collarR * 0.10f, centre.y - collarR * 0.10f, true);
            g.setGradientFill(hl);
            fillEllipseCentred(g, centre, collarR);
        }

        // Black domed centre (~50 % diameter)
        const float domeR = r * 0.50f;
        {
            juce::ColourGradient dome(juce::Colour(0xFF3A343A),
                                      centre.x - domeR * 0.22f, centre.y - domeR * 0.40f,
                                      juce::Colour(0xFF100D12),
                                      centre.x + domeR * 0.45f, centre.y + domeR * 0.55f, true);
            g.setGradientFill(dome);
            fillEllipseCentred(g, centre, domeR);
        }

        // Fine cool-white pointer
        const float thk = std::max(1.2f, r * 0.030f);
        const auto pointerCol = hovered ? juce::Colour(c::knobPointerHot)
                                        : juce::Colour(0xFFE8F0F4);
        drawPointerLine(g, centre, angle, domeR * 0.25f, r * 0.94f, pointerCol, thk);

        // Centre dot
        const float dotR = std::max(1.5f, r * 0.06f);
        g.setColour(juce::Colour(0xFFCFC8CC));
        fillEllipseCentred(g, centre, dotR);

        hoverHalo(g, centre, r, hovered);
    }

    // Dispatcher — picks paint routine from the slider's property bag.
    KnobStyle resolveStyle(juce::Slider& slider)
    {
        const auto& props = slider.getProperties();
        const auto v = props.getWithDefault(knobStyleProperty,
                                            static_cast<int>(KnobStyle::SovietFluted));
        return static_cast<KnobStyle>(static_cast<int>(v));
    }
}

RatosaurusLookAndFeel::RatosaurusLookAndFeel()
{
    auto& assets = AssetCache::getInstance();
    knobBase  = assets.createDrawable(AssetCache::Asset::KnobBase);
    knobRotor = assets.createDrawable(AssetCache::Asset::KnobRotor);

    setColourScheme(juce::LookAndFeel_V4::getDarkColourScheme());

    setColour(juce::ResizableWindow::backgroundColourId, theme::col(c::panelMid));
    setColour(juce::PopupMenu::backgroundColourId,       theme::col(c::lcdBgBottom));
    setColour(juce::PopupMenu::textColourId,             theme::col(c::amberMid));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, theme::col(c::amberLow));
    setColour(juce::PopupMenu::highlightedTextColourId,  theme::col(c::amberHigh));
}

// ---------------------------------------------------------------------------
//  drawRotarySlider — dispatcher. Each style implementation lives in the
//  anonymous namespace above.
// ---------------------------------------------------------------------------
void RatosaurusLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                             int x, int y, int width, int height,
                                             float sliderPos,
                                             float startAngle, float endAngle,
                                             juce::Slider& slider)
{
    const auto fullBounds = juce::Rectangle<float>(static_cast<float>(x),
                                                   static_cast<float>(y),
                                                   static_cast<float>(width),
                                                   static_cast<float>(height));
    const float pad = juce::jmax(2.0f, fullBounds.getWidth() * 0.06f);
    const auto bounds = fullBounds.reduced(pad);

    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    if (knobBase != nullptr && knobRotor != nullptr)
    {
        knobBase->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);

        const float scale = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 512.0f;
        const float drawW = 512.0f * scale;
        const float drawH = 512.0f * scale;
        const float tx = bounds.getCentreX() - drawW * 0.5f;
        const float ty = bounds.getCentreY() - drawH * 0.5f;
        const auto transform = juce::AffineTransform::rotation(angle, 256.0f, 256.0f)
                                   .scaled(scale)
                                   .translated(tx, ty);
        knobRotor->draw(g, 1.0f, transform);
        return;
    }

    paintSovietFluted(g, bounds, angle, slider.isMouseOverOrDragging());
}

// ---------------------------------------------------------------------------
//  ComboBox — LCD-style amber readout on dark recess
// ---------------------------------------------------------------------------
void RatosaurusLookAndFeel::drawComboBox(juce::Graphics& g,
                                         int width, int height,
                                         bool /*isButtonDown*/,
                                         int /*buttonX*/, int /*buttonY*/,
                                         int /*buttonW*/, int /*buttonH*/,
                                         juce::ComboBox& /*box*/)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f,
                                         static_cast<float>(width),
                                         static_cast<float>(height)).reduced(0.5f);

    g.setColour(theme::col(c::lcdBgBottom));
    g.fillRoundedRectangle(bounds, 2.0f);

    g.setColour(theme::col(c::lcdBorder));
    g.drawRoundedRectangle(bounds, 2.0f, 0.6f);

    // Down-arrow indicator on the right
    juce::Path arrow;
    const float ay = static_cast<float>(height) * 0.5f - 2.5f;
    const float ax = static_cast<float>(width) - 12.0f;
    arrow.addTriangle(ax,       ay,
                      ax + 7.0f, ay,
                      ax + 3.5f, ay + 5.0f);
    g.setColour(theme::col(c::amberMid));
    g.fillPath(arrow);
}

void RatosaurusLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(6, 0, box.getWidth() - 22, box.getHeight());
    label.setFont(getComboBoxFont(box));
    label.setColour(juce::Label::textColourId, theme::col(c::lcdText));
}

// ---------------------------------------------------------------------------
//  Label — engraved-brass effect
// ---------------------------------------------------------------------------
void RatosaurusLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    if (label.isBeingEdited())
        return; // editor paints itself

    const auto area = label.getLocalBounds();
    const auto text = label.getText();
    const auto justification = label.getJustificationType();
    const auto font = getLabelFont(label);

    g.setFont(font);

    // Dark recess shadow — 1 px down
    g.setColour(theme::col(c::wellBlack).withAlpha(0.55f));
    g.drawText(text, area.translated(0, 1), justification, true);

    // Foreground engraved tone — respect external textColourId override
    g.setColour(label.findColour(juce::Label::textColourId));
    g.drawText(text, area, justification, true);
}

// ---------------------------------------------------------------------------
//  Buttons — brushed-metal with brass outline
// ---------------------------------------------------------------------------
void RatosaurusLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                 juce::Button& button,
                                                 const juce::Colour& backgroundColour,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);

    auto fill = backgroundColour;
    if (shouldDrawButtonAsDown)
        fill = fill.darker(0.20f);
    else if (shouldDrawButtonAsHighlighted)
        fill = fill.brighter(0.10f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 2.0f);

    g.setColour(theme::col(c::brassPatina));
    g.drawRoundedRectangle(bounds, 2.0f, 0.6f);
}

void RatosaurusLookAndFeel::drawButtonText(juce::Graphics& g,
                                           juce::TextButton& button,
                                           bool /*shouldDrawButtonAsHighlighted*/,
                                           bool /*shouldDrawButtonAsDown*/)
{
    g.setFont(getTextButtonFont(button, button.getHeight()));

    const auto& colourId = button.getToggleState()
        ? juce::TextButton::textColourOnId
        : juce::TextButton::textColourOffId;

    g.setColour(button.findColour(colourId));
    g.drawFittedText(button.getButtonText(),
                     button.getLocalBounds(),
                     juce::Justification::centred,
                     1);
}

// ---------------------------------------------------------------------------
//  Fonts
// ---------------------------------------------------------------------------
juce::Font RatosaurusLookAndFeel::getLabelFont(juce::Label& label)
{
    auto base = label.getFont();
    return Fonts::oswald(base.getHeight() > 1.0f ? base.getHeight() : f::engravedLabel, true);
}

juce::Font RatosaurusLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return Fonts::oswald(f::lcdReadout, true);
}

juce::Font RatosaurusLookAndFeel::getPopupMenuFont()
{
    return Fonts::oswald(f::lcdReadout, true);
}

juce::Font RatosaurusLookAndFeel::getTextButtonFont(juce::TextButton&, int)
{
    return Fonts::oswald(f::engravedLabel + 1.0f, true);
}

} // namespace rato

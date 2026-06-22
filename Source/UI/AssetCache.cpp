#include "AssetCache.h"
#include "BinaryData.h"

namespace rato
{

AssetCache& AssetCache::getInstance()
{
    static AssetCache instance;
    return instance;
}

bool AssetCache::isAssetAvailable(Asset key) const noexcept
{
    const auto entry = getBinaryDataFor(key);
    return entry.data != nullptr && entry.size > 0;
}

std::unique_ptr<juce::Drawable> AssetCache::createDrawable(Asset key)
{
    auto* master = getOrCreateMaster(key);
    if (master == nullptr)
        return nullptr;

    return master->createCopy();
}

void AssetCache::clear() noexcept
{
    for (auto& m : masters)
        m.reset();
}

juce::Drawable* AssetCache::getOrCreateMaster(Asset key)
{
    const auto index = static_cast<size_t>(key);
    if (index >= kNumAssets)
        return nullptr;

    if (masters[index] != nullptr)
        return masters[index].get();

    const auto entry = getBinaryDataFor(key);
    if (entry.data == nullptr || entry.size <= 0)
        return nullptr;

    auto parsed = juce::Drawable::createFromImageData(entry.data,
                                                      static_cast<size_t>(entry.size));
    if (parsed == nullptr)
        return nullptr;

    auto* raw = parsed.get();
    masters[index] = std::move(parsed);
    return raw;
}

// ---------------------------------------------------------------------------
//  Binary data lookup
//  Skull entry intentionally empty — no placeholder created in Stage 10C.
//  Real centerpiece art lands in a later iteration.
// ---------------------------------------------------------------------------
AssetCache::BinaryEntry AssetCache::getBinaryDataFor(Asset key) noexcept
{
    switch (key)
    {
        case Asset::Logo:
            return { BinaryData::rato_logo_svg,        BinaryData::rato_logo_svgSize };
        case Asset::Signature:
            return { BinaryData::signature_shchur_svg, BinaryData::signature_shchur_svgSize };
        case Asset::IconOff:
            return { BinaryData::off_svg,              BinaryData::off_svgSize };
        case Asset::IconTape:
            return { BinaryData::tape_svg,             BinaryData::tape_svgSize };
        case Asset::IconTube:
            return { BinaryData::tube_svg,             BinaryData::tube_svgSize };
        case Asset::IconFet:
            return { BinaryData::fet_svg,              BinaryData::fet_svgSize };
        case Asset::IconRat:
            return { BinaryData::rat_svg,              BinaryData::rat_svgSize };
        case Asset::Faceplate:
            return { BinaryData::faceplate_v4_svg,     BinaryData::faceplate_v4_svgSize };
        case Asset::KnobBase:
            return { BinaryData::knob_base_v4_svg,     BinaryData::knob_base_v4_svgSize };
        case Asset::KnobRotor:
            return { BinaryData::knob_rotor_v4_svg,    BinaryData::knob_rotor_v4_svgSize };

        case Asset::Skull:
        case Asset::NumAssets:
        default:
            return {};
    }
}

} // namespace rato

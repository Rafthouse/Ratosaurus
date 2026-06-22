#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <memory>

namespace rato
{

/**
    Singleton cache for SVG assets bundled via juce_add_binary_data.

    Stage 10C: real BinaryData symbols wired through getBinaryDataFor().
    Master Drawables are parsed lazily on first request and cached as
    std::array<unique_ptr, kNumAssets> indexed by the Asset enum value.

    createDrawable() returns a clone of the master so callers get their
    own owning juce::Drawable (Drawable carries per-component transform
    state and can't safely be shared).

    Thread model: GUI thread only. No locking — the audio thread never
    touches this class. Singleton storage is C++11 thread-safe at first
    call (Meyer's), but subsequent access is single-thread.
*/
class AssetCache
{
public:
    enum class Asset
    {
        Logo       = 0,
        Skull      = 1,
        Signature  = 2,
        IconOff    = 3,
        IconTape   = 4,
        IconTube   = 5,
        IconFet    = 6,
        IconRat    = 7,
        Faceplate  = 8,
        KnobBase   = 9,
        KnobRotor  = 10,
        NumAssets  = 11
    };

    static AssetCache& getInstance();

    bool isAssetAvailable(Asset key) const noexcept;

    /** Returns a cloned juce::Drawable for caller ownership.
        Returns nullptr if the asset has no BinaryData entry yet (e.g. Skull
        in Stage 10C) or if parsing fails. */
    std::unique_ptr<juce::Drawable> createDrawable(Asset key);

    /** Releases all cached masters. Next createDrawable() call re-parses. */
    void clear() noexcept;

private:
    AssetCache() = default;
    ~AssetCache() = default;

    struct BinaryEntry
    {
        const char* data = nullptr;
        int size = 0;
    };

    static BinaryEntry getBinaryDataFor(Asset key) noexcept;

    juce::Drawable* getOrCreateMaster(Asset key);

    static constexpr size_t kNumAssets = static_cast<size_t>(Asset::NumAssets);

    std::array<std::unique_ptr<juce::Drawable>, kNumAssets> masters;

    JUCE_DECLARE_NON_COPYABLE(AssetCache)
};

} // namespace rato

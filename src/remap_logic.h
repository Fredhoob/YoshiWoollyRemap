#pragma once

#include <stdint.h>

// Wii Remote *core* button bits from wut's <padscore/wpad.h>.
// These do NOT apply to Classic/Pro extension button bitmasks.
namespace YoshiRemap {
constexpr uint32_t kA = 0x0800u;
constexpr uint32_t kB = 0x0400u;
constexpr uint32_t kAB = kA | kB;

// Idempotent: a physical A or B becomes virtual A, and B disappears.
static inline uint32_t MapHeld(uint32_t buttons) {
    return (buttons & ~kAB) | ((buttons & kAB) ? kA : 0u);
}

// KPAD supplies three separate edge bitmasks. Recompute A's combined
// transition instead of ORing A/B releases, which would produce a false
// A-release when physical A is held and physical B is released.
static inline void MapKpad(uint32_t &hold, uint32_t &trigger, uint32_t &release) {
    const uint32_t h = hold;
    const uint32_t t = trigger;
    const uint32_t r = release;

    const bool aNow = (h & kA) != 0;
    const bool bNow = (h & kB) != 0;

    // A button was down before this sample if it is still held without
    // a new press, or if it was just released.
    const bool aBefore = (aNow && (t & kA) == 0) || (r & kA) != 0;
    const bool bBefore = (bNow && (t & kB) == 0) || (r & kB) != 0;

    const bool combinedNow = aNow || bNow;
    const bool combinedBefore = aBefore || bBefore;

    hold = (h & ~kAB) | (combinedNow ? kA : 0u);
    trigger = (t & ~kAB) | ((combinedNow && !combinedBefore) ? kA : 0u);
    release = (r & ~kAB) | ((!combinedNow && combinedBefore) ? kA : 0u);

    // Preserve a full press-and-release that both occurred inside one read.
    // KPAD can report both flags even when the final hold bit is clear.
    if (!combinedNow && (t & kAB) && (r & kAB)) {
        trigger |= kA;
        release |= kA;
    }
}
}  // namespace YoshiRemap

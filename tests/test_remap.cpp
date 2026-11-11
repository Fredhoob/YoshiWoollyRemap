#include "../src/remap_logic.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
using YoshiRemap::kA;
using YoshiRemap::kB;

static void Check(uint32_t h, uint32_t t, uint32_t r, uint32_t expectedH, uint32_t expectedT, uint32_t expectedR) {
    YoshiRemap::MapKpad(h, t, r);
    if (!(h == expectedH && t == expectedT && r == expectedR)) { std::fprintf(stderr, "FAILED actual h=%04x t=%04x r=%04x expected h=%04x t=%04x r=%04x\n", h, t, r, expectedH, expectedT, expectedR); }
    assert(h == expectedH && t == expectedT && r == expectedR);
}

int main() {
    // Raw WPAD: A still A, B becomes A, 1/2 and D-pad unchanged.
    assert(YoshiRemap::MapHeld(0) == 0);
    assert(YoshiRemap::MapHeld(kA) == kA);
    assert(YoshiRemap::MapHeld(kB) == kA);
    assert(YoshiRemap::MapHeld(kA | kB) == kA);
    assert(YoshiRemap::MapHeld(kB | 0x0100u) == (kA | 0x0100u));
    // KPAD: B press becomes A press.
    Check(kB, kB, 0, kA, kA, 0);
    // KPAD: B hold does not create a fresh press.
    Check(kB, 0, 0, kA, 0, 0);
    // KPAD: B release becomes A release.
    Check(0, 0, kB, 0, 0, kA);
    // KPAD: Pressing B while A is already held should NOT trigger A again.
    Check(kA | kB, kB, 0, kA, 0, 0);
    // KPAD: Releasing B while A stays down should NOT release A.
    Check(kA, 0, kB, kA, 0, 0);
    // KPAD: Releasing A while B stays down should NOT release A.
    Check(kB, 0, kA, kA, 0, 0);
    // KPAD: Pressing A while B is already down should NOT re-trigger A.
    Check(kA | kB, kA, 0, kA, 0, 0);
    // KPAD: Maintain other buttons.
    Check(kB | 0x0100u, kB | 0x0100u, 0, kA | 0x0100u, kA | 0x0100u, 0);
    // KPAD: Quick tap reported pressed and released in the same sample.
    Check(0, kB, kB, 0, kA, kA);
    // Exhaust all valid previous/current combinations of two independent buttons.
    for (uint32_t prev = 0; prev < 4; ++prev) {
        for (uint32_t curr = 0; curr < 4; ++curr) {
            uint32_t prevBits = ((prev & 1u) ? kA : 0u) | ((prev & 2u) ? kB : 0u);
            uint32_t currBits = ((curr & 1u) ? kA : 0u) | ((curr & 2u) ? kB : 0u);
            uint32_t trigBits = currBits & ~prevBits;
            uint32_t relBits  = prevBits & ~currBits;
            uint32_t expHeld = currBits ? kA : 0u;
            uint32_t expTrig = (currBits && !prevBits) ? kA : 0u;
            uint32_t expRel  = (!currBits && prevBits) ? kA : 0u;
            Check(currBits, trigBits, relBits, expHeld, expTrig, expRel);
        }
    }
    // Idempotence (if both KPAD and WPAD hooks run).
    uint32_t h = kB, t = kB, r = 0;
    YoshiRemap::MapKpad(h, t, r);
    YoshiRemap::MapKpad(h, t, r);
    assert(h == kA && t == kA && r == 0);
}

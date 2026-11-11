// Yoshi's Woolly World: make a horizontal/bare Wii Remote B trigger act as A.
// Only the 3 native Wii U title IDs are enabled. GamePad, Nunchuk, Classic,
// and Pro controllers are never remapped.

#include <coreinit/title.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <wups.h>

#include <stdint.h>

#include "remap_logic.h"

WUPS_PLUGIN_NAME("Yoshi Woolly World - Wiimote B as A");
WUPS_PLUGIN_DESCRIPTION("Only in Yoshi's Woolly World: bare Wii Remote B performs A; original A still works.");
WUPS_PLUGIN_VERSION("0.1.0");
WUPS_PLUGIN_AUTHOR("ChatGPT (source project; not console-tested)");
WUPS_PLUGIN_LICENSE("MIT");

static bool gIsWoollyWorld = false;

static bool IsWoollyWorldTitle(uint64_t title) {
    switch (title) {
        case 0x0005000010131F00ULL:  // Japan  AY C J
        case 0x0005000010184D00ULL:  // USA    AY C E
        case 0x0005000010184E00ULL:  // Europe AY C P
            return true;
        default:
            return false;
    }
}

// A bare Wii Remote is the controller used for this game's sideways mode.
// Orientation itself is not a separate controller type. Exclude any Nunchuk,
// Classic Controller, Pro Controller, and their MotionPlus variants.
static bool IsBareWiimote(uint8_t extensionType) {
    return extensionType == WPAD_EXT_CORE || extensionType == WPAD_EXT_MPLUS;
}

ON_APPLICATION_START() {
    gIsWoollyWorld = IsWoollyWorldTitle(OSGetTitleID());
}

ON_APPLICATION_ENDS() {
    gIsWoollyWorld = false;
}

static void RemapKpadSamples(KPADStatus *data, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        KPADStatus &sample = data[i];
        if (IsBareWiimote(sample.extensionType) && sample.error == WPAD_ERROR_NONE) {
            YoshiRemap::MapKpad(sample.hold, sample.trigger, sample.release);
        }
    }
}

DECL_FUNCTION(uint32_t, KPADRead, KPADChan channel, KPADStatus *data, uint32_t count) {
    const uint32_t read = real_KPADRead(channel, data, count);
    if (gIsWoollyWorld && data && read > 0) {
        RemapKpadSamples(data, read < count ? read : count);
    }
    return read;
}

DECL_FUNCTION(uint32_t, KPADReadEx, KPADChan channel, KPADStatus *data, uint32_t count, KPADError *outError) {
    const uint32_t read = real_KPADReadEx(channel, data, count, outError);
    if (gIsWoollyWorld && data && read > 0) {
        RemapKpadSamples(data, read < count ? read : count);
    }
    return read;
}

// Some games use raw Wii Remote reports, rather than the calibrated KPAD path.
// 'buttons' in this struct is the CORE Wii Remote bitfield, not Pro/Classic.
static void RemapWpadStatus(WPADStatus *status) {
    if (status && status->error == WPAD_ERROR_NONE && IsBareWiimote(status->extensionType)) {
        status->buttons = static_cast<uint16_t>(YoshiRemap::MapHeld(status->buttons));
    }
}

DECL_FUNCTION(void, WPADRead, WPADChan channel, WPADStatus *status) {
    real_WPADRead(channel, status);
    if (gIsWoollyWorld) {
        RemapWpadStatus(status);
    }
}

// The unified raw-ring-buffer path is separate from KPADRead and WPADRead.
DECL_FUNCTION(void, KPADGetUnifiedWpadStatus, KPADChan channel, KPADUnifiedWpadStatus *buffer, uint32_t count) {
    real_KPADGetUnifiedWpadStatus(channel, buffer, count);
    if (gIsWoollyWorld && buffer) {
        for (uint32_t i = 0; i < count; ++i) {
            RemapWpadStatus(&buffer[i].core);
        }
    }
}

WUPS_MUST_REPLACE_FOR_PROCESS(KPADRead, WUPS_LOADER_LIBRARY_PADSCORE, KPADRead, WUPS_FP_TARGET_PROCESS_GAME);
WUPS_MUST_REPLACE_FOR_PROCESS(KPADReadEx, WUPS_LOADER_LIBRARY_PADSCORE, KPADReadEx, WUPS_FP_TARGET_PROCESS_GAME);
WUPS_MUST_REPLACE_FOR_PROCESS(WPADRead, WUPS_LOADER_LIBRARY_PADSCORE, WPADRead, WUPS_FP_TARGET_PROCESS_GAME);
WUPS_MUST_REPLACE_FOR_PROCESS(KPADGetUnifiedWpadStatus, WUPS_LOADER_LIBRARY_PADSCORE, KPADGetUnifiedWpadStatus, WUPS_FP_TARGET_PROCESS_GAME);

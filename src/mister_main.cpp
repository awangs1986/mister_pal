//
//  PAL MiSTer hybrid frontend
//
//  Modes:
//    (default / -testpattern)  Color-bar 320x224 (NeoGeo size) â†?DDR3
//    -game <dir>               Reserved: launch SDLPAL against game data
//
//  Adapted from MiSTerOrganize/MiSTer_PICO-8 â€?GPL-3.0
//

#include "native_video_writer.h"
#include "pal_mister_input.h"
#include "pal_mister_game.h"
#include "pal_mister_autoplay.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/file.h>
#include <thread>
#include <unistd.h>
#include <vector>

/* Only one ./PAL may own the DDR audio ring â€?dual writers corrupt PCM. */
static int g_singleton_fd = -1;
static bool pal_acquire_singleton(void) {
    g_singleton_fd = open("/var/run/PAL.singleton.lock", O_CREAT | O_RDWR, 0644);
    if (g_singleton_fd < 0) {
        fprintf(stderr, "PAL: cannot open singleton lock\n");
        return false;
    }
    if (flock(g_singleton_fd, LOCK_EX | LOCK_NB) != 0) {
        fprintf(stderr, "PAL: another ./PAL already holds singleton lock â€?exit\n");
        close(g_singleton_fd);
        g_singleton_fd = -1;
        return false;
    }
    return true;
}

static const int W = 320;
static const int H = 224; /* NeoGeo active â€?full-frame bars, not 200+pad */

// NeoGeo native 320Ã—224 fill. Last band used to be black (40px) which looked
// like "bars shoved left" on a full-width CRT â€?use a bright band instead.
// Bright columns at x=0 and x=319 show true left/right edges vs overscan.
static void fill_colorbar(std::vector<uint8_t>& rgba) {
    rgba.assign(W * H * 4, 0);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int i = (y * W + x) * 4;
            int band = x * 8 / W;
            uint8_t r = 0, g = 0, b = 0;
            switch (band) {
                case 0: r = 255; g = 255; b = 255; break;
                case 1: r = 255; g = 255; b = 0;   break;
                case 2: r = 0;   g = 255; b = 255; break;
                case 3: r = 0;   g = 255; b = 0;   break;
                case 4: r = 255; g = 0;   b = 255; break;
                case 5: r = 255; g = 0;   b = 0;   break;
                case 6: r = 0;   g = 0;   b = 255; break;
                default: r = 255; g = 128; b = 0;  break; /* bright orange, was black */
            }
            /* Edge beacons â€?overwrite band color */
            if (x == 0 || x == W - 1) {
                r = 255; g = 255; b = 255;
            }
            rgba[i + 0] = r;
            rgba[i + 1] = g;
            rgba[i + 2] = b;
            rgba[i + 3] = 255;
        }
    }
}

static void keepalive_thread() {
    // FPGA clears frame_ready after ~30 stale vblanks (~0.5s) without a new
    // frame_counter. Keepalive bumps the counter only (no buffer rewrite).
    while (NativeVideoWriter_IsActive()) {
        NativeVideoWriter_KeepaliveTick();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

static void run_testpattern() {
    std::vector<uint8_t> frame;
    fill_colorbar(frame);
    fprintf(stderr, "PAL: testpattern 320x224 NeoGeo (bright last band + x=0/319 edges) + keepalive\n");

    NativeVideoWriter_WriteFrame(frame.data(), W, H);

    std::thread ka(keepalive_thread);
    ka.detach();

    PAL_MisterInput inp;
    PAL_MisterInput_Reset(&inp);
    fprintf(stderr, "PAL: input map P1 D-pad + Space/Enter/ESC/R/A/Q/F/S/D/E (OSD remappable)\n");

    while (true) {
        PAL_MisterInput_Poll(&inp, 0);
        if (inp.pressed) {
            fprintf(stderr, "PAL: keys pressed=0x%08X held=0x%08X dir=%d joy=0x%08X\n",
                    inp.pressed, inp.held, inp.dir,
                    NativeVideoWriter_ReadJoystick(0));
        }
        /* Quit: ESC + Space together (Menu + Search) */
        if ((inp.held & (PAL_MKEY_MENU | PAL_MKEY_SEARCH)) ==
            (PAL_MKEY_MENU | PAL_MKEY_SEARCH)) {
            fprintf(stderr, "PAL: quit combo (ESC+Space)\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

int main(int argc, char** argv) {
    setvbuf(stderr, nullptr, _IONBF, 0);
    fprintf(stderr, "PAL hybrid ARM frontend\n");

    if (!pal_acquire_singleton()) {
        return 1;
    }

    bool testpattern = true;
    bool autoplay = false;
    const char* game_dir = nullptr;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-nativevideo") || !strcmp(argv[i], "-testpattern")) {
            testpattern = true;
        } else if (!strcmp(argv[i], "-game") && i + 1 < argc) {
            game_dir = argv[++i];
            testpattern = false;
        } else if (!strcmp(argv[i], "-data") && i + 1 < argc) {
            game_dir = argv[++i];
        } else if (!strcmp(argv[i], "-autoplay")) {
            autoplay = true;
        }
    }

    if (!NativeVideoWriter_Init()) {
        fprintf(stderr, "PAL: NativeVideoWriter_Init failed (need /dev/mem on MiSTer)\n");
        return 1;
    }

    if (!testpattern && game_dir) {
        if (autoplay) {
            PAL_MisterAuto_SetEnabled(1);
        }
        int rc = PAL_Mister_RunGame(game_dir);
        NativeVideoWriter_ClearScreen();
        NativeVideoWriter_Shutdown();
        unlink("/media/fat/config/PAL2.s0");
        unlink("/media/fat/config/PAL.s0");
        _exit(rc);
    }

    run_testpattern();

    NativeVideoWriter_ClearScreen();
    NativeVideoWriter_Shutdown();
    unlink("/media/fat/config/PAL2.s0");
    unlink("/media/fat/config/PAL.s0");
    _exit(0);
}

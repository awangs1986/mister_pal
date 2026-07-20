//
//  MiSTer DDR joystick → SDLPAL key bits
//

#include "pal_mister_input.h"
#include "native_video_writer.h"
#include "mister_diag.h"

enum { kDirSouth = 0, kDirWest = 1, kDirNorth = 2, kDirEast = 3, kDirUnknown = 4 };

void PAL_MisterInput_Reset(PAL_MisterInput* st) {
    if (!st) return;
    st->held = 0;
    st->pressed = 0;
    st->released = 0;
    st->dir = kDirUnknown;
}

static uint32_t joy_to_keys(uint32_t joy) {
    uint32_t k = 0;
    if (joy & (1u << PAL_JOY_RIGHT)) k |= PAL_MKEY_RIGHT;
    if (joy & (1u << PAL_JOY_LEFT))  k |= PAL_MKEY_LEFT;
    if (joy & (1u << PAL_JOY_DOWN))  k |= PAL_MKEY_DOWN;
    if (joy & (1u << PAL_JOY_UP))    k |= PAL_MKEY_UP;
    if (joy & (1u << PAL_JOY_SPACE)) k |= PAL_MKEY_SEARCH;
    if (joy & (1u << PAL_JOY_ENTER)) k |= PAL_MKEY_SEARCH;
    /* Current PAL.sv jn/jp maps Start→Enter (bit5), Select→ESC (bit6). */
    if (joy & (1u << PAL_JOY_ESC))   k |= PAL_MKEY_MENU;
    if (joy & (1u << PAL_JOY_R))     k |= PAL_MKEY_REPEAT;
    if (joy & (1u << PAL_JOY_A))     k |= PAL_MKEY_AUTO;
    /* Do NOT map joy Q/Y → Flee. On SNES layout Y was Flee → QuitGame →
     * PAL_Shutdown → handler relaunch = bounce to opening menu after CG. */
    if (joy & (1u << PAL_JOY_F))     k |= PAL_MKEY_FORCE;
    if (joy & (1u << PAL_JOY_S))     k |= PAL_MKEY_STATUS;
    if (joy & (1u << PAL_JOY_D))     k |= PAL_MKEY_DEFEND;
    if (joy & (1u << PAL_JOY_E))     k |= PAL_MKEY_USEITEM;
    return k;
}

static int dir_from_keys(uint32_t held) {
    /* Last-wins priority matching common RPG feel: prefer vertical then horizontal
       is wrong for sdlpal — sdlpal uses order stamps. For held pad, use
       Up > Down > Left > Right priority when multiple (simple & stable). */
    if (held & PAL_MKEY_UP)    return kDirNorth;
    if (held & PAL_MKEY_DOWN)  return kDirSouth;
    if (held & PAL_MKEY_LEFT)  return kDirWest;
    if (held & PAL_MKEY_RIGHT) return kDirEast;
    return kDirUnknown;
}

void PAL_MisterInput_FromJoy(PAL_MisterInput* st, uint32_t joy) {
    if (!st) return;
    uint32_t now = joy_to_keys(joy);
    st->pressed  = now & ~st->held;
    st->released = st->held & ~now;
    st->held     = now;
    st->dir      = dir_from_keys(now);
}

void PAL_MisterInput_Poll(PAL_MisterInput* st, int player) {
    if (!st) return;
    uint32_t joy = NativeVideoWriter_ReadJoystick(player);
    PAL_MisterInput_FromJoy(st, joy);
    PAL_Diag_Input(joy, st->held, st->pressed, st->released);
}

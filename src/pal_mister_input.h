//
//  MiSTer DDR joystick → SDLPAL key bits
//
//  CONF_STR (PAL.sv):
//    D-pad: bit0=Right bit1=Left bit2=Down bit3=Up
//    J1,Space,Enter,ESC,R,A,Q,F,S,D,E → bits 4..13
//
//  Same PALKEY bit values as sdlpal/input.h (keep in sync).
//

#ifndef PAL_MISTER_INPUT_H
#define PAL_MISTER_INPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Match sdlpal/input.h enum PALKEY */
enum {
    PAL_MKEY_MENU      = (1u << 0),  /* ESC */
    PAL_MKEY_SEARCH    = (1u << 1),  /* Space / Enter */
    PAL_MKEY_DOWN      = (1u << 2),
    PAL_MKEY_LEFT      = (1u << 3),
    PAL_MKEY_UP        = (1u << 4),
    PAL_MKEY_RIGHT     = (1u << 5),
    PAL_MKEY_REPEAT    = (1u << 8),  /* R */
    PAL_MKEY_AUTO      = (1u << 9),  /* A */
    PAL_MKEY_DEFEND    = (1u << 10), /* D */
    PAL_MKEY_USEITEM   = (1u << 11), /* E */
    PAL_MKEY_FLEE      = (1u << 13), /* Q */
    PAL_MKEY_STATUS    = (1u << 14), /* S */
    PAL_MKEY_FORCE     = (1u << 15), /* F */
};

/* MiSTer joystick_N bit indices (hps_io) */
enum {
    PAL_JOY_RIGHT = 0,
    PAL_JOY_LEFT  = 1,
    PAL_JOY_DOWN  = 2,
    PAL_JOY_UP    = 3,
    PAL_JOY_SPACE = 4,
    PAL_JOY_ENTER = 5,
    PAL_JOY_ESC   = 6,
    PAL_JOY_R     = 7,
    PAL_JOY_A     = 8,
    PAL_JOY_Q     = 9,
    PAL_JOY_F     = 10,
    PAL_JOY_S     = 11,
    PAL_JOY_D     = 12,
    PAL_JOY_E     = 13,
};

/* dir: 0=South 1=West 2=North 3=East 4=Unknown (palcommon PALDIRECTION) */
typedef struct {
    uint32_t held;       /* keys currently down (level) */
    uint32_t pressed;    /* rising-edge this poll (one-shot) */
    uint32_t released;   /* falling-edge this poll */
    int      dir;        /* 0..3 or 4=unknown */
} PAL_MisterInput;

void PAL_MisterInput_Reset(PAL_MisterInput* st);

/* Read player 0..3 from DDR via NativeVideoWriter_ReadJoystick. */
void PAL_MisterInput_Poll(PAL_MisterInput* st, int player);

/* Map raw MiSTer joy word → update st (edge + level + dir). */
void PAL_MisterInput_FromJoy(PAL_MisterInput* st, uint32_t joy);

#ifdef __cplusplus
}
#endif

#endif

#ifndef PAL_NATIVE_VIDEO_WRITER_H
#define PAL_NATIVE_VIDEO_WRITER_H

//
//  Native Video DDR3 Writer for SDLPAL / PAL MiSTer hybrid core
//
//  DDR framebuffer is NeoGeo standard 320x224 RGB565.
//  Callers may pass 320x200; writer letterboxes with black bars
//  (12 + 200 + 12) before the flip — output is already 320x224.
//
//  Copyright (C) 2026 — GPL-3.0 (adapted from MiSTerOrganize/MiSTer_PICO-8)
//

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NV_OUT_WIDTH   320
#define NV_OUT_HEIGHT  224   /* NeoGeo NTSC active */
#define NV_SRC_HEIGHT  200   /* SDLPAL / testpattern content */
#define NV_V_PAD       ((NV_OUT_HEIGHT - NV_SRC_HEIGHT) / 2)  /* 12 */

bool NativeVideoWriter_Init(void);
void NativeVideoWriter_Shutdown(void);

/// Convert RGBA8888 → RGB565 into inactive DDR3 buffer (pad to 320x224), then flip.
/// Accepts width=320 and height=200 (padded) or height=224 (copy as-is).
void NativeVideoWriter_WriteFrame(const void* rgba8_pixels, int width, int height);

/// Write RGB565; same height rules as WriteFrame.
void NativeVideoWriter_WriteFrameRGB565(const uint16_t* rgb565, int width, int height);

bool NativeVideoWriter_IsActive(void);
void NativeVideoWriter_KeepaliveTick(void);
void NativeVideoWriter_ClearScreen(void);

uint32_t NativeVideoWriter_CheckCart(void);
uint32_t NativeVideoWriter_ReadCart(void* buf, uint32_t max_size);
void NativeVideoWriter_AckCart(void);

/// Joystick bitmask: bit0=R,1=L,2=D,3=U,4=B,5=A,6=Y,7=X,8=Start,...
uint32_t NativeVideoWriter_ReadJoystick(int player);
uint32_t NativeVideoWriter_ReadFeedback(void);
uint32_t NativeVideoWriter_ReadSavestate(void);

static inline uint8_t  NV_SsCmd (uint32_t w) { return (uint8_t)( w        & 0xFFu); }
static inline uint8_t  NV_SsSlot(uint32_t w) { return (uint8_t)((w >> 8 ) & 0xFFu); }
static inline uint8_t  NV_SsSeq (uint32_t w) { return (uint8_t)((w >> 16) & 0xFFu); }
static inline uint32_t NV_FeedbackVblankCounter(uint32_t fb) { return fb >> 2; }
static inline uint32_t NV_FeedbackBufferStatus(uint32_t fb) { return fb & 3; }

uint32_t NativeVideoWriter_AudioSpace(void);
void NativeVideoWriter_WriteAudio(const int16_t *stereo_samples, uint32_t num_samples);

/* Dump currently displayed buffer (ctrl bit0) as binary PPM P6 to FILE*. */
int NativeVideoWriter_DumpFramePPM(FILE* fp);

#ifdef __cplusplus
}
#endif

#endif

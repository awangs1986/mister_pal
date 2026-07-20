//
//  Native Video DDR3 Writer — PAL (SDLPAL) MiSTer hybrid
//
//  DDR3 Memory Map @ 0x3A000000:
//    +0x000  Control (frame_counter[31:2] | active_buf[1:0])
//    +0x008  Joystick P1
//    +0x010  Cart ctrl
//    +0x018  VSync feedback
//    +0x020  Audio write ptr
//    +0x028  Audio read ptr
//    +0x030/+0x038/+0x040  Joystick P2/P3/P4
//    +0x048  Savestate ctrl
//    +0x100  Framebuffer 0  (320*224*2 = 0x23000)
//    +0x24000 Framebuffer 1
//    +0x48000 Audio ring (4096 stereo S16)
//    +0x50000 Cart data region
//
//  Output size is always NeoGeo 320x224. 320x200 sources are padded
//  with black bars (12+200+12) in software before flip.
//
//  Adapted from MiSTerOrganize/MiSTer_PICO-8 — GPL-3.0
//

#include "native_video_writer.h"
#include "mister_diag.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define NV_DDR_PHYS_BASE     0x3A000000u
#define NV_DDR_REGION_SIZE   0x00100000u   /* 1MB */
#define NV_CTRL_OFFSET       0x00000000u
#define NV_JOY0_OFFSET       0x00000008u
#define NV_CART_CTRL_OFFSET  0x00000010u
#define NV_FEEDBACK_OFFSET   0x00000018u
#define NV_AUD_WPTR_OFFSET   0x00000020u
#define NV_AUD_RPTR_OFFSET   0x00000028u
#define NV_JOY1_OFFSET       0x00000030u
#define NV_JOY2_OFFSET       0x00000038u
#define NV_JOY3_OFFSET       0x00000040u
#define NV_SS_OFFSET         0x00000048u
#define NV_BUF0_OFFSET       0x00000100u
#define NV_BUF1_OFFSET       0x00024000u
#define NV_AUD_RING_OFFSET   0x00048000u
#define NV_CART_DATA_OFFSET  0x00050000u
#define NV_CART_MAX_SIZE     0x00040000u
#define NV_AUD_RING_SAMPLES  4096
#define NV_AUD_RING_MASK     (NV_AUD_RING_SAMPLES - 1)
#define NV_FRAME_WIDTH       NV_OUT_WIDTH
#define NV_FRAME_HEIGHT      NV_OUT_HEIGHT
#define NV_FRAME_BYTES       (NV_FRAME_WIDTH * NV_FRAME_HEIGHT * 2)
#define NV_LINE_BYTES        (NV_FRAME_WIDTH * 2)

static int mem_fd = -1;
static volatile uint8_t* ddr_base = NULL;
static uint32_t frame_counter = 0;
static int active_buf = 0;

bool NativeVideoWriter_Init(void) {
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("NativeVideoWriter: open /dev/mem");
        return false;
    }

    ddr_base = (volatile uint8_t*)mmap(NULL, NV_DDR_REGION_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, NV_DDR_PHYS_BASE);
    if (ddr_base == MAP_FAILED) {
        perror("NativeVideoWriter: mmap");
        ddr_base = NULL;
        close(mem_fd);
        mem_fd = -1;
        return false;
    }

    memset((void*)(ddr_base + NV_BUF0_OFFSET), 0, NV_FRAME_BYTES);
    memset((void*)(ddr_base + NV_BUF1_OFFSET), 0, NV_FRAME_BYTES);
    *(volatile uint32_t*)(ddr_base + NV_CTRL_OFFSET) = 0;
    *(volatile uint32_t*)(ddr_base + NV_CART_CTRL_OFFSET) = 0;
    *(volatile uint32_t*)(ddr_base + NV_FEEDBACK_OFFSET) = 0;
    *(volatile uint32_t*)(ddr_base + NV_AUD_WPTR_OFFSET) = 0;
    *(volatile uint32_t*)(ddr_base + NV_AUD_RPTR_OFFSET) = 0;
    memset((void*)(ddr_base + NV_AUD_RING_OFFSET), 0, NV_AUD_RING_SAMPLES * 4);
    *(volatile uint32_t*)(ddr_base + NV_JOY0_OFFSET) = 0;
    *(volatile uint32_t*)(ddr_base + NV_JOY1_OFFSET) = 0;
    *(volatile uint32_t*)(ddr_base + NV_JOY2_OFFSET) = 0;
    *(volatile uint32_t*)(ddr_base + NV_JOY3_OFFSET) = 0;
    frame_counter = 0;
    active_buf = 0;

    fprintf(stderr, "NativeVideoWriter[PAL]: mapped 0x%08X, out %dx%d (%d bytes), pad %d+%d+%d\n",
            NV_DDR_PHYS_BASE, NV_FRAME_WIDTH, NV_FRAME_HEIGHT, NV_FRAME_BYTES,
            NV_V_PAD, NV_SRC_HEIGHT, NV_V_PAD);
    return true;
}

void NativeVideoWriter_Shutdown(void) {
    if (ddr_base) {
        *(volatile uint32_t*)(ddr_base + NV_CTRL_OFFSET) = 0;
        munmap((void*)ddr_base, NV_DDR_REGION_SIZE);
        ddr_base = NULL;
    }
    if (mem_fd >= 0) {
        close(mem_fd);
        mem_fd = -1;
    }
}

static void nv_flip(void) {
    /* Publish only after the whole inactive buffer is visible to the FPGA. */
    __sync_synchronize();
    frame_counter++;
    *(volatile uint32_t*)(ddr_base + NV_CTRL_OFFSET) =
        (frame_counter << 2) | (active_buf & 1);
    active_buf ^= 1;
}

/*
 * Returns 1 when inactive buffer is safe to overwrite.
 * Returns 0 on timeout — caller MUST drop the present (no tear).
 * Also paces to ≤1 flip per CRT field via feedback vblank_counter.
 */
static int nv_wait_until_safe_to_present(void) {
    if (!ddr_base)
        return 0;
    if (frame_counter < 2)
        return 1;

    /*
     * Feedback word: {vblank_counter[31:2], active_buffer[1], 0}.
     * After nv_flip, active_buf is the NEXT write target; the buffer we just
     * published is (active_buf^1). Wait until FPGA reports displaying that.
     */
    const uint32_t want_disp = (uint32_t)((active_buf ^ 1) & 1);
    uint32_t feedback = 0;
    int i;

    for (i = 0; i < 400; i++) { /* ~100 ms */
        feedback = *(volatile uint32_t*)(ddr_base + NV_FEEDBACK_OFFSET);
        if (((feedback >> 1) & 1u) == want_disp)
            break;
        usleep(250);
    }
    if (i >= 400) {
        PAL_Diag_VideoWait(1, feedback, want_disp, i);
        return 0; /* drop — rewriting mid-scan causes heavy jitter */
    }
    if (i >= 64)
        PAL_Diag_VideoWait(0, feedback, want_disp, i);

    /* Pace: at most one publish per CRT field (vblank_counter must advance). */
    {
        static uint32_t s_last_vb;
        uint32_t vb0 = feedback >> 2;
        if (frame_counter >= 3 && vb0 == s_last_vb) {
            for (int j = 0; j < 200; j++) {
                feedback = *(volatile uint32_t*)(ddr_base + NV_FEEDBACK_OFFSET);
                if ((feedback >> 2) != vb0)
                    break;
                usleep(250);
            }
        }
        s_last_vb = *(volatile uint32_t*)(ddr_base + NV_FEEDBACK_OFFSET) >> 2;
    }
    return 1;
}

static volatile uint16_t* nv_inactive_buf(void) {
    uint32_t buf_offset = (active_buf == 0) ? NV_BUF0_OFFSET : NV_BUF1_OFFSET;
    return (volatile uint16_t*)(ddr_base + buf_offset);
}

static void nv_clear_buf(volatile uint16_t* dst) {
    memset((void*)dst, 0, NV_FRAME_BYTES);
}

void NativeVideoWriter_WriteFrameRGB565(const uint16_t* rgb565, int width, int height) {
    if (!ddr_base || !rgb565 || width != NV_FRAME_WIDTH)
        return;
    if (height != NV_SRC_HEIGHT && height != NV_FRAME_HEIGHT)
        return;

    if (!nv_wait_until_safe_to_present())
        return;

    volatile uint16_t* dst = nv_inactive_buf();

    if (height == NV_FRAME_HEIGHT) {
        memcpy((void*)dst, rgb565, NV_FRAME_BYTES);
    } else {
        /* 320x200 → 320x224 black letterbox */
        nv_clear_buf(dst);
        memcpy((void*)(dst + NV_V_PAD * NV_FRAME_WIDTH),
               rgb565, (size_t)NV_SRC_HEIGHT * NV_LINE_BYTES);
    }
    nv_flip();
    PAL_Diag_VideoPresent(width, height, 0);
}

void NativeVideoWriter_WriteFrame(const void* rgba8_pixels, int width, int height) {
    if (!ddr_base || !rgba8_pixels || width != NV_FRAME_WIDTH)
        return;
    if (height != NV_SRC_HEIGHT && height != NV_FRAME_HEIGHT)
        return;

    /* Convert off-DDR then one memcpy — avoids long clear/fill tearing on CRT
     * during high-rate RNG / fade animations. */
    static uint16_t linebuf[NV_FRAME_WIDTH * NV_FRAME_HEIGHT];
    const uint8_t* src = (const uint8_t*)rgba8_pixels;
    int y0 = (height == NV_FRAME_HEIGHT) ? 0 : NV_V_PAD;

    if (height == NV_SRC_HEIGHT) {
        /* Keep letterbox black without wiping the active 200 lines twice. */
        memset(linebuf, 0, (size_t)NV_V_PAD * NV_LINE_BYTES);
        memset(linebuf + (NV_V_PAD + NV_SRC_HEIGHT) * NV_FRAME_WIDTH, 0,
               (size_t)NV_V_PAD * NV_LINE_BYTES);
    }

    for (int y = 0; y < height; y++) {
        uint16_t* dst = linebuf + (y0 + y) * NV_FRAME_WIDTH;
        for (int x = 0; x < NV_FRAME_WIDTH; x++) {
            int si = (y * NV_FRAME_WIDTH + x) * 4;
            uint8_t r = src[si + 0];
            uint8_t g = src[si + 1];
            uint8_t b = src[si + 2];
            dst[x] = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
        }
    }

    if (!nv_wait_until_safe_to_present())
        return;

    volatile uint16_t* dst = nv_inactive_buf();
    memcpy((void*)dst, linebuf, NV_FRAME_BYTES);
    nv_flip();
    PAL_Diag_VideoPresent(width, height, 0);
}

bool NativeVideoWriter_IsActive(void) { return ddr_base != NULL; }

void NativeVideoWriter_KeepaliveTick(void) {
    if (!ddr_base) return;
    frame_counter++;
    *(volatile uint32_t*)(ddr_base + NV_CTRL_OFFSET) =
        (frame_counter << 2) | ((active_buf ^ 1) & 1);
}

void NativeVideoWriter_ClearScreen(void) {
    if (!ddr_base) return;
    memset((void*)(ddr_base + NV_BUF0_OFFSET), 0, NV_FRAME_BYTES);
    memset((void*)(ddr_base + NV_BUF1_OFFSET), 0, NV_FRAME_BYTES);
    nv_flip();
}

uint32_t NativeVideoWriter_CheckCart(void) {
    if (!ddr_base) return 0;
    uint32_t val = *(volatile uint32_t*)(ddr_base + NV_CART_CTRL_OFFSET);
    return (val > NV_CART_MAX_SIZE) ? 0 : val;
}

uint32_t NativeVideoWriter_ReadCart(void* buf, uint32_t max_size) {
    if (!ddr_base || !buf) return 0;
    uint32_t file_size = NativeVideoWriter_CheckCart();
    if (file_size == 0) return 0;
    if (file_size > max_size) file_size = max_size;
    if (file_size > NV_CART_MAX_SIZE) file_size = NV_CART_MAX_SIZE;
    memcpy(buf, (const void*)(ddr_base + NV_CART_DATA_OFFSET), file_size);
    return file_size;
}

void NativeVideoWriter_AckCart(void) {
    if (!ddr_base) return;
    *(volatile uint32_t*)(ddr_base + NV_CART_CTRL_OFFSET) = 0;
}

uint32_t NativeVideoWriter_ReadJoystick(int player) {
    if (!ddr_base || player < 0 || player > 3) return 0;
    static const uint32_t joy_offsets[4] = {
        NV_JOY0_OFFSET, NV_JOY1_OFFSET, NV_JOY2_OFFSET, NV_JOY3_OFFSET
    };
    return *(volatile uint32_t*)(ddr_base + joy_offsets[player]);
}

uint32_t NativeVideoWriter_ReadFeedback(void) {
    if (!ddr_base) return 0;
    return *(volatile uint32_t*)(ddr_base + NV_FEEDBACK_OFFSET);
}

uint32_t NativeVideoWriter_ReadSavestate(void) {
    if (!ddr_base) return 0;
    return *(volatile uint32_t*)(ddr_base + NV_SS_OFFSET);
}

uint32_t NativeVideoWriter_AudioSpace(void) {
    if (!ddr_base) return 0;
    uint32_t w = *(volatile uint32_t*)(ddr_base + NV_AUD_WPTR_OFFSET) & NV_AUD_RING_MASK;
    uint32_t r = *(volatile uint32_t*)(ddr_base + NV_AUD_RPTR_OFFSET) & NV_AUD_RING_MASK;
    uint32_t used = (w - r) & NV_AUD_RING_MASK;
    return NV_AUD_RING_SAMPLES - 1 - used;
}

void NativeVideoWriter_WriteAudio(const int16_t *stereo_samples, uint32_t num_samples) {
    if (!ddr_base || !stereo_samples || num_samples == 0) return;

    /* Clamp to free space once — never re-query mid-loop with a stale wptr
     * (that could wrap and overwrite unread samples → DAC noise). */
    uint32_t space = NativeVideoWriter_AudioSpace();
    if (space == 0) return;
    if (num_samples > space) num_samples = space;

    volatile uint32_t* wptr_reg = (volatile uint32_t*)(ddr_base + NV_AUD_WPTR_OFFSET);
    volatile int16_t* ring = (volatile int16_t*)(ddr_base + NV_AUD_RING_OFFSET);
    uint32_t wp = *wptr_reg & NV_AUD_RING_MASK;

    for (uint32_t i = 0; i < num_samples; i++) {
        uint32_t idx = (wp + i) & NV_AUD_RING_MASK;
        ring[idx * 2 + 0] = stereo_samples[i * 2 + 0];
        ring[idx * 2 + 1] = stereo_samples[i * 2 + 1];
    }

    __sync_synchronize();
    *wptr_reg = (wp + num_samples) & NV_AUD_RING_MASK;
}

int NativeVideoWriter_DumpFramePPM(FILE* fp) {
    if (!ddr_base || !fp)
        return 0;

    uint32_t ctrl = *(volatile uint32_t*)(ddr_base + NV_CTRL_OFFSET);
    int shown = (int)(ctrl & 1u);
    const volatile uint16_t* src = (const volatile uint16_t*)(ddr_base +
        (shown ? NV_BUF1_OFFSET : NV_BUF0_OFFSET));

    if (fprintf(fp, "P6\n%d %d\n255\n", NV_FRAME_WIDTH, NV_FRAME_HEIGHT) < 0)
        return 0;

    for (int i = 0; i < NV_FRAME_WIDTH * NV_FRAME_HEIGHT; i++) {
        uint16_t p = src[i];
        uint8_t r = (uint8_t)(((p >> 11) & 0x1F) << 3);
        uint8_t g = (uint8_t)(((p >> 5) & 0x3F) << 2);
        uint8_t b = (uint8_t)((p & 0x1F) << 3);
        if (fputc(r, fp) == EOF || fputc(g, fp) == EOF || fputc(b, fp) == EOF)
            return 0;
    }
    fflush(fp);
    return 1;
}

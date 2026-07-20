//
//  MiSTer PAL2 diagnostic logger
//

#include "mister_diag.h"
#include "pal_mister_input.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

enum {
    JOY_ENTER = (1u << 5), /* Start → confirm/Search */
    JOY_ESC   = (1u << 6), /* Select → Menu */
    JOY_Q     = (1u << 9), /* Flee → QuitGame */
};

static int g_ready;
static uint32_t g_last_joy;
static uint32_t g_last_held;
static uint64_t g_t0_ms;

static uint32_t g_present_n;
static uint32_t g_wait_to_n;
static uint32_t g_wait_long_n;
static uint32_t g_drop_n;
static uint64_t g_stats_ms;

static uint64_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
}

static void emit(const char* tag, const char* fmt, va_list ap)
{
    uint64_t t = now_ms() - g_t0_ms;
    fprintf(stderr, "[DIAG][+%llums][%s] ", (unsigned long long)t, tag ? tag : "?");
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    fflush(stderr);
}

void PAL_Diag_Log(const char* tag, const char* fmt, ...)
{
    if (!g_ready) return;
    va_list ap;
    va_start(ap, fmt);
    emit(tag, fmt, ap);
    va_end(ap);
}

void PAL_Diag_Init(void)
{
    g_t0_ms = now_ms();
    g_ready = 1;
    g_last_joy = 0;
    g_last_held = 0;
    g_present_n = 0;
    g_wait_to_n = 0;
    g_wait_long_n = 0;
    g_stats_ms = g_t0_ms;

    fprintf(stderr,
        "[DIAG][+0ms][boot] === PAL2 diagnostic logger ===\n"
        "[DIAG][+0ms][boot] map: joy bit5(Start/Enter)=Search  bit6(Select/ESC)=Menu  bit9(Q)=Flee/Quit\n"
        "[DIAG][+0ms][boot] bounce: Menu opens in-game menu; Flee/Quit→PAL_Shutdown→exit→handler restart looks like title\n"
        "[DIAG][+0ms][boot] flicker: watch [vid] wait_TIMEOUT / presents/s bursts\n"
        "[DIAG][+0ms][boot] grep: [DIAG]\n");
    fflush(stderr);
}

static void keys_brief(char* out, size_t n, uint32_t k)
{
    out[0] = 0;
    if (k & PAL_MKEY_MENU)   strncat(out, "MENU ", n - strlen(out) - 1);
    if (k & PAL_MKEY_SEARCH) strncat(out, "SEARCH ", n - strlen(out) - 1);
    if (k & PAL_MKEY_FLEE)   strncat(out, "FLEE ", n - strlen(out) - 1);
    if (k & PAL_MKEY_UP)     strncat(out, "U ", n - strlen(out) - 1);
    if (k & PAL_MKEY_DOWN)   strncat(out, "D ", n - strlen(out) - 1);
    if (k & PAL_MKEY_LEFT)   strncat(out, "L ", n - strlen(out) - 1);
    if (k & PAL_MKEY_RIGHT)  strncat(out, "R ", n - strlen(out) - 1);
    if (k & PAL_MKEY_STATUS) strncat(out, "STATUS ", n - strlen(out) - 1);
    if (k & PAL_MKEY_AUTO)   strncat(out, "AUTO ", n - strlen(out) - 1);
    if (k & PAL_MKEY_REPEAT) strncat(out, "REPEAT ", n - strlen(out) - 1);
    if (k & PAL_MKEY_FORCE)  strncat(out, "FORCE ", n - strlen(out) - 1);
    if (k & PAL_MKEY_DEFEND) strncat(out, "DEFEND ", n - strlen(out) - 1);
    if (k & PAL_MKEY_USEITEM)strncat(out, "ITEM ", n - strlen(out) - 1);
    if (!out[0]) strncat(out, "-", n - 1);
}

void PAL_Diag_Input(uint32_t raw_joy, uint32_t held, uint32_t pressed, uint32_t released)
{
    if (!g_ready) return;

    const int both_start_select = ((raw_joy & JOY_ENTER) && (raw_joy & JOY_ESC));
    const int interesting_edge =
        (pressed & (PAL_MKEY_MENU | PAL_MKEY_SEARCH | PAL_MKEY_FLEE)) != 0 ||
        (released & (PAL_MKEY_MENU | PAL_MKEY_SEARCH | PAL_MKEY_FLEE)) != 0 ||
        ((raw_joy ^ g_last_joy) & (JOY_ENTER | JOY_ESC | JOY_Q)) != 0;

    if (both_start_select)
    {
        PAL_Diag_Log("input",
            "CONFLICT Start+Select same poll joy=0x%08X held=0x%08X (if one physical button, mapping wrong)",
            raw_joy, held);
    }

    if (!interesting_edge && !both_start_select)
    {
        g_last_joy = raw_joy;
        g_last_held = held;
        return;
    }

    char pb[96], rb[96], hb[96];
    keys_brief(pb, sizeof(pb), pressed);
    keys_brief(rb, sizeof(rb), released);
    keys_brief(hb, sizeof(hb), held);

    PAL_Diag_Log("input",
        "joy=0x%08X bits[5Start=%d 6Select=%d 9Q=%d] pressed={%s} released={%s} held={%s}",
        raw_joy,
        (raw_joy & JOY_ENTER) ? 1 : 0,
        (raw_joy & JOY_ESC) ? 1 : 0,
        (raw_joy & JOY_Q) ? 1 : 0,
        pb, rb, hb);

    g_last_joy = raw_joy;
    g_last_held = held;
    (void)g_last_held;
}

void PAL_Diag_VideoWait(int timed_out, uint32_t feedback, uint32_t expected_buf, int wait_iters)
{
    if (!g_ready) return;
    if (timed_out)
    {
        g_wait_to_n++;
        g_drop_n++;
        PAL_Diag_Log("vid",
            "wait_TIMEOUT DROP iters=%d fb=0x%X expect_buf=%u (no tear)",
            wait_iters, feedback, expected_buf);
        return;
    }
    if (wait_iters >= 64)
    { /* ~16ms+ */
        g_wait_long_n++;
        PAL_Diag_Log("vid", "wait_long iters=%d fb=0x%X expect_buf=%u",
            wait_iters, feedback, expected_buf);
    }
}

void PAL_Diag_VideoPresent(int width, int height, int partial)
{
    if (!g_ready) return;
    g_present_n++;

    uint64_t t = now_ms();
    if (t - g_stats_ms >= 1000)
    {
        PAL_Diag_Log("vid",
            "stats presents/s=%u wait_timeouts=%u wait_long=%u drops=%u last=%dx%d%s",
            g_present_n, g_wait_to_n, g_wait_long_n, g_drop_n,
            width, height, partial ? " partial" : "");
        g_present_n = 0;
        g_wait_to_n = 0;
        g_wait_long_n = 0;
        g_drop_n = 0;
        g_stats_ms = t;
    }
}

void PAL_Diag_VideoFlip(uint32_t frame_counter, int active_buf)
{
    (void)frame_counter;
    (void)active_buf;
    /* Rate covered by VideoPresent stats; keep hook for future. */
}

void PAL_Diag_Lifecycle(const char* event)
{
    if (!g_ready) return;
    PAL_Diag_Log("life", "%s", event ? event : "?");
}

void PAL_Diag_Shutdown(const char* reason, int exit_code)
{
    if (!g_ready)
    {
        fprintf(stderr, "[DIAG][shutdown] reason=%s code=%d (pre-init)\n",
            reason ? reason : "?", exit_code);
        fflush(stderr);
        return;
    }
    PAL_Diag_Log("shutdown",
        "reason=%s exit_code=%d → process exit; handler will relaunch → looks like title bounce",
        reason ? reason : "?", exit_code);
}

void PAL_Diag_Music(int track, int loop)
{
    if (!g_ready) return;
    /* Opening menu = 4, title splash = 5 in classic RIX numbering. */
    if (track == 4 || track == 5 || track == 0)
    {
        PAL_Diag_Log("music", "track=%d loop=%d%s",
            track, loop,
            track == 4 ? " (OPENING_MENU)" :
            track == 5 ? " (TITLE)" :
            track == 0 ? " (STOP)" : "");
    }
}

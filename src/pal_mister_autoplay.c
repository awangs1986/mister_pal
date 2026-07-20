//
//  MiSTer autoplay harness — reach 李大娘「皮痒」line, dump frame, freeze.
//

#include "pal_mister_autoplay.h"
#include "native_video_writer.h"
#include "mister_diag.h"

#include <stdio.h>
#include <unistd.h>
#include <wchar.h>

static int g_auto_on;
static int g_captured;

void PAL_MisterAuto_SetEnabled(int on)
{
    g_auto_on = on ? 1 : 0;
    if (g_auto_on)
        PAL_Diag_Log("auto", "AUTOPLAY on — New Game → seek dialog 皮痒 → screenshot freeze");
}

int PAL_MisterAuto_Enabled(void)
{
    return g_auto_on;
}

int PAL_MisterAuto_ForceNewGame(void)
{
    return g_auto_on;
}

int PAL_MisterAuto_ShouldSkipMedia(void)
{
    return g_auto_on;
}

int PAL_MisterAuto_ShouldSkipWait(void)
{
    if (!g_auto_on || g_captured)
        return 0;
    return 1;
}

static int text_has_piyang(const wchar_t* text)
{
    static const wchar_t needle[] = { 0x76AE, 0x75D2, 0 };
    if (!text || !text[0])
        return 0;
    return wcsstr(text, needle) != NULL;
}

void PAL_MisterAuto_OnDialogText(const wchar_t* text)
{
    if (!g_auto_on || g_captured || !text)
        return;

    if (!text_has_piyang(text))
        return;

    g_captured = 1;
    PAL_Diag_Log("auto", "MATCH dialog contains 皮痒 — capturing and freezing");
    PAL_Diag_Lifecycle("AUTOPLAY success: 皮痒 dialog");

    usleep(80000);

    FILE* fp = fopen("/tmp/pal_piyang.ppm", "wb");
    if (fp) {
        if (NativeVideoWriter_DumpFramePPM(fp))
            PAL_Diag_Log("auto", "wrote /tmp/pal_piyang.ppm");
        fclose(fp);
    }
    fp = fopen("/media/fat/logs/PAL2/pal_piyang.ppm", "wb");
    if (fp) {
        NativeVideoWriter_DumpFramePPM(fp);
        fclose(fp);
    }
    {
        FILE* m = fopen("/tmp/pal_piyang.ok", "w");
        if (m) {
            fputs("ok\n", m);
            fclose(m);
        }
    }

    for (;;)
        sleep(3600);
}

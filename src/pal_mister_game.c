//
//  SDLPAL game orchestration for MiSTer hybrid ARM frontend.
//

#include "pal_mister_game.h"
#include "pal_mister_autoplay.h"

#include "native_video_writer.h"
#include "mister_diag.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"
#include "palcfg.h"
#include "game.h"

extern VOID PAL_Init(VOID);
extern VOID PAL_SplashScreen(VOID);

int PAL_Mister_RunGame(const char* game_dir)
{
    char* argv_local[1];

    if (!game_dir || !game_dir[0]) {
        fprintf(stderr, "PAL: -game requires a directory\n");
        return 1;
    }

    if (chdir(game_dir) != 0) {
        perror("PAL: chdir(game_dir)");
        return 1;
    }

    PAL_Diag_Init();
    fprintf(stderr, "PAL: SDLPAL game mode dir=%s\n", game_dir);
    fflush(stderr);

    /* Prefer dummy drivers before SDL_Init (no X11 on MiSTer ARM). */
    fprintf(stderr, "PAL: setenv drivers\n");
    fflush(stderr);
    setenv("SDL_VIDEODRIVER", "dummy", 1);
    setenv("SDL_AUDIODRIVER", "dummy", 1);

    /* No keepalive during game: WriteFrame already flips; keepalive raced
     * the inactive buffer and caused CRT flicker. */

    fprintf(stderr, "PAL: SDL_Init...\n");
    fflush(stderr);
    if (SDL_Init(PAL_SDL_INIT_FLAGS) == SDL_FAIL) {
        fprintf(stderr, "PAL: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    fprintf(stderr, "PAL: SDL_Init ok\n");
    fflush(stderr);

    PAL_LoadConfig(TRUE);
    fprintf(stderr, "PAL: config loaded\n");
    fflush(stderr);

    free(gConfig.pszGamePath);
    gConfig.pszGamePath = strdup(".");
    free(gConfig.pszSavePath);
    gConfig.pszSavePath = strdup(".");
    gConfig.fLaunchSetting = FALSE;
    gConfig.fFullScreen = FALSE;
    gConfig.dwScreenWidth = 320;
    gConfig.dwScreenHeight = 200;
    gConfig.fEnableGLSL = FALSE;
    gConfig.fUseTouchOverlay = FALSE;
    /* AVI playback (esp. 3.avi) has hung the ARM frontend; keep off unless needed.
     * OpeningMenu also skips 3.avi on MiSTer. Autoplayskip uses ShouldSkipMedia. */
    gConfig.fEnableAviPlay = FALSE;
    gConfig.iSampleRate = 48000;
    gConfig.iAudioChannels = 2;
    gConfig.iMusicVolume = 70;
    gConfig.iSoundVolume = 70;
    gConfig.eMusicType = MUSIC_OGG;

    argv_local[0] = (char*)"PAL";
    if (UTIL_Platform_Init(1, argv_local) != 0) {
        fprintf(stderr, "PAL: UTIL_Platform_Init failed\n");
        SDL_Quit();
        return 1;
    }

    PAL_Diag_Lifecycle("PAL_Init begin");
    fprintf(stderr, "PAL: PAL_Init...\n");
    fflush(stderr);
    PAL_Init();
    PAL_Diag_Lifecycle("splash begin");
    fprintf(stderr, "PAL: splash...\n");
    fflush(stderr);
    /* Skip trademark AVI on CRT bring-up; splash is the first milestone. */
    PAL_SplashScreen();
    PAL_Diag_Lifecycle("GameMain begin (opening menu next)");
    fprintf(stderr, "PAL: GameMain\n");
    fflush(stderr);
    PAL_GameMain();

    PAL_Diag_Shutdown("GameMain returned", 0);
    PAL_Shutdown(0);
    return 0;
}

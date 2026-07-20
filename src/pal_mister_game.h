//
//  Launch SDLPAL against a game data directory (MKF folder).
//

#ifndef PAL_MISTER_GAME_H
#define PAL_MISTER_GAME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Returns process-style exit code. Does not return on normal game exit
 * (PAL_Shutdown longjmp / exit). NativeVideoWriter must already be Init'd. */
int PAL_Mister_RunGame(const char* game_dir);

#ifdef __cplusplus
}
#endif

#endif

/* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// MiSTer hybrid port of SDLPAL — NeoGeo 320×224 DDR path.
// Content stays 320×200; NativeVideoWriter letterboxes to 224.
//

#ifndef PAL_CONFIG_H
#define PAL_CONFIG_H

#define PAL_HAS_OGG                    1
#define PAL_HAS_OPUS                   0
#define PAL_HAS_MP3                    0
#define PAL_HAS_NATIVEMIDI             0
#define PAL_HAS_JOYSTICKS              0
#define PAL_HAS_SDLCD                  0
#define PAL_HAS_GLSL                   0
#define PAL_HAS_CONFIG_PAGE            0
#define PAL_HAS_PLATFORM_SPECIFIC_UTILS 1
#define PAL_NO_LAUNCH_UI               1
#define PAL_SCALE_SCREEN               TRUE

#define PAL_PREFIX                     "./"
#define PAL_SAVE_PREFIX                "./"

#define PAL_DEFAULT_WINDOW_WIDTH       320
#define PAL_DEFAULT_WINDOW_HEIGHT      200
#define PAL_DEFAULT_TEXTURE_WIDTH      320
#define PAL_DEFAULT_TEXTURE_HEIGHT     200
#define PAL_DEFAULT_FULLSCREEN_HEIGHT  200

#if SDL_VERSION_ATLEAST(2, 0, 0)
# define PAL_VIDEO_INIT_FLAGS  0
#else
# define PAL_VIDEO_INIT_FLAGS  (SDL_SWSURFACE)
#endif

#define PAL_SDL_INIT_FLAGS     (SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE)

#define PAL_PLATFORM           "MiSTer"
#define PAL_CREDIT             NULL
#define PAL_PORTYEAR           "2026"

#include <sys/time.h>
#include <ctype.h>

#endif

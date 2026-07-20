cmake_minimum_required(VERSION 3.13)
project(MiSTer_PAL C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-Ofast -DNDEBUG")
add_compile_options(-Wno-unused-parameter -Wno-sign-compare -Wno-unused-function)
add_definitions(-D__MISTER__=1)

option(PAL_WITH_SDLPAL "Link SDLPAL engine for -game mode" ON)

# Prefer in-tree sdlpal/ (GitHub layout); fall back to sibling ../sdlpal for local trees.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sdlpal/main.c")
    set(SDLPAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/sdlpal")
else()
    set(SDLPAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../sdlpal")
endif()

set(PAL_SOURCES
    src/mister_main.cpp
    src/native_video_writer.c
    src/pal_mister_input.c
    src/mister_diag.c
    src/pal_mister_autoplay.c
)

if(PAL_WITH_SDLPAL)
    list(APPEND PAL_SOURCES
        src/pal_mister_game.c
        src/pal_mister_platform.c
    )

    # Core engine (exclude GLSL / GL loader)
    set(SDLPAL_ENGINE
        audio.c aviplay.c battle.c ending.c fight.c font.c game.c global.c
        input.c itemmenu.c magicmenu.c main.c map.c midi.c midi_timidity.c
        midi_tsf.c mp3play.c oggplay.c opusplay.c overlay.c palcfg.c
        palcommon.c paldebug.c palette.c play.c res.c resampler.c rngplay.c
        scene.c script.c sound.c rixplay.cpp text.c ui.c uibattle.c uigame.c util.c
        video.c yj1.c
    )
    foreach(f ${SDLPAL_ENGINE})
        list(APPEND PAL_SOURCES "${SDLPAL_ROOT}/${f}")
    endforeach()

    list(APPEND PAL_SOURCES "${SDLPAL_ROOT}/sdl_compat/sdl_compat.c")

    file(GLOB ADPLUG_SRC "${SDLPAL_ROOT}/adplug/*.cpp" "${SDLPAL_ROOT}/adplug/*.c")
    list(APPEND PAL_SOURCES ${ADPLUG_SRC})

    # Embedded libogg + libvorbis (decode) for MUSIC_OGG
    file(GLOB OGGVORBIS_SRC "${SDLPAL_ROOT}/liboggvorbis/src/*.c")
    list(APPEND PAL_SOURCES ${OGGVORBIS_SRC})

    # Embedded Timidity used by midi_timidity.c
    set(SDLPAL_TIMIDITY
        common.c instrum.c mix.c output.c playmidi.c readmidi.c
        resample.c tables.c timidity.c
    )
    foreach(f ${SDLPAL_TIMIDITY})
        list(APPEND PAL_SOURCES "${SDLPAL_ROOT}/timidity/${f}")
    endforeach()

    # Native MIDI common only (no ALSA); synth stays disabled via pal_config
    list(APPEND PAL_SOURCES "${SDLPAL_ROOT}/native_midi/native_midi_common.c")
endif()

add_executable(PAL ${PAL_SOURCES})
target_include_directories(PAL PRIVATE src)

if(PAL_WITH_SDLPAL)
    target_compile_definitions(PAL PRIVATE
        SDL_LOADSO_DISABLED=1
        SDL_DYNAMIC_API=0
        USE_SDL3=0
        PAL_HAS_GIT_REVISION=0
    )
    # MiSTer pal_config.h must win over unix/
    target_include_directories(PAL PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mister_pal"
        "${SDLPAL_ROOT}"
        "${SDLPAL_ROOT}/sdl_compat"
        "${SDLPAL_ROOT}/adplug"
        "${SDLPAL_ROOT}/native_midi"
        "${SDLPAL_ROOT}/timidity"
        "${SDLPAL_ROOT}/liboggvorbis/include"
        "${SDLPAL_ROOT}/liboggvorbis/src"
    )

    # SDL2: prefer explicit prefix (cross-built static), else pkg-config / find_package
    if(DEFINED SDL2_STATIC_PREFIX AND EXISTS "${SDL2_STATIC_PREFIX}/lib/libSDL2.a")
        message(STATUS "SDL2 static prefix: ${SDL2_STATIC_PREFIX}")
        target_include_directories(PAL PRIVATE "${SDL2_STATIC_PREFIX}/include" "${SDL2_STATIC_PREFIX}/include/SDL2")
        target_link_libraries(PAL PRIVATE "${SDL2_STATIC_PREFIX}/lib/libSDL2.a")
    elseif(DEFINED ENV{SDL2_STATIC_PREFIX} AND EXISTS "$ENV{SDL2_STATIC_PREFIX}/lib/libSDL2.a")
        message(STATUS "SDL2 static prefix (env): $ENV{SDL2_STATIC_PREFIX}")
        target_include_directories(PAL PRIVATE "$ENV{SDL2_STATIC_PREFIX}/include" "$ENV{SDL2_STATIC_PREFIX}/include/SDL2")
        target_link_libraries(PAL PRIVATE "$ENV{SDL2_STATIC_PREFIX}/lib/libSDL2.a")
    else()
        find_package(SDL2 QUIET)
        if(SDL2_FOUND)
            target_link_libraries(PAL PRIVATE SDL2::SDL2)
        else()
            find_library(SDL2_LIB NAMES SDL2 libSDL2.a PATHS
                /usr/lib/arm-linux-gnueabihf
                /usr/arm-linux-gnueabihf/lib
                /usr/lib
            )
            find_path(SDL2_INC SDL.h PATH_SUFFIXES SDL2)
            if(NOT SDL2_LIB OR NOT SDL2_INC)
                message(FATAL_ERROR
                    "PAL_WITH_SDLPAL=ON but SDL2 not found. Build/install armhf static SDL2 and pass "
                    "-DSDL2_STATIC_PREFIX=/path (expects lib/libSDL2.a and include/SDL2)")
            endif()
            target_include_directories(PAL PRIVATE "${SDL2_INC}")
            target_link_libraries(PAL PRIVATE "${SDL2_LIB}")
        endif()
    endif()
endif()

target_link_libraries(PAL PRIVATE pthread m)

# MiSTer rootfs is glibc 2.31; host armhf toolchain is newer �?static link.
# Do NOT link libdl: static+dlopen triggers _dl_call_libc_early_init abort on device.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_C_COMPILER MATCHES "arm-linux")
    target_link_options(PAL PRIVATE -static)
endif()

install(TARGETS PAL RUNTIME DESTINATION games/PAL)
install(TARGETS PAL RUNTIME DESTINATION games/PAL2)
install(PROGRAMS games/PAL/_handler.sh DESTINATION games/PAL)
install(PROGRAMS games/PAL2/_handler.sh DESTINATION games/PAL2)

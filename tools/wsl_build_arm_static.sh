#!/usr/bin/env bash
set -euxo pipefail
cd "/mnt/d/godot project/240pal/MiSTer_PAL"
export SDL2_STATIC_PREFIX="$HOME/sdl2-armhf-static"
cmake -S . -B build-arm-static \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=arm \
  -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
  -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ \
  -DCMAKE_C_FLAGS="-mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon" \
  -DCMAKE_CXX_FLAGS="-mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon" \
  -DCMAKE_EXE_LINKER_FLAGS=-static \
  -DSDL2_STATIC_PREFIX="$SDL2_STATIC_PREFIX" \
  -DPAL_WITH_SDLPAL=ON
cmake --build build-arm-static -j"$(nproc)"
ls -la build-arm-static/PAL
file build-arm-static/PAL
cp -f build-arm-static/PAL games/PAL/PAL
chmod +x games/PAL/PAL
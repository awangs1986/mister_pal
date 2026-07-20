#!/bin/bash
set -euxo pipefail
cd "/mnt/d/godot project/240pal/MiSTer_PAL/build-arm-sdlpal"
make clean
make -j$(nproc)
cp -f PAL ../games/PAL2/PAL
cp -f PAL ../games/PAL/PAL
chmod +x ../games/PAL2/PAL ../games/PAL/PAL
ls -la PAL ../games/PAL2/PAL
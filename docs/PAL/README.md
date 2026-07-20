# PAL MiSTer Hybrid Core (SDLPAL)

ARM runs the game engine; FPGA outputs **320×240 @ ~15.75 kHz** with **320×200 content + 20-line letterbox**.

## Status

| Piece | State |
|-------|--------|
| DDR3 native video writer (320×200 RGB565) | Done |
| `pal_video_timing.sv` 320×240 / 15 kHz | Done (PLL retune in Quartus) |
| `pal_video_reader.sv` letterbox | Done (ported from PICO-8) |
| ARM testpattern binary | Buildable |
| Quartus `.rbf` | Needs Quartus Prime Lite **17.0** on a Windows/Linux build box |
| Full SDLPAL game loop on ARM | Next (after testpattern CRT pass) |

## Deploy (SD card)

```
/media/fat/_Other/PAL_YYYYMMDD.rbf
/media/fat/games/PAL/PAL
/media/fat/games/PAL/_handler.sh
/media/fat/games/PAL/Games/          # 98 soft MKF later
/media/fat/logs/PAL/
```

Requires [MiSTer Frontier](https://github.com/MiSTerOrganize/MiSTer_Frontier) Master_Daemon.

## Build ARM

```bash
cd MiSTer_PAL
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon" \
  -DCMAKE_CXX_FLAGS="-mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon"
make -j$(nproc)
cp PAL ../games/PAL/
```

## Build FPGA

1. Open `fpga/PAL.qpf` in Quartus 17.0
2. Retune PLL so `ce_pix` ≈ **6.552 MHz** (416×15750)
3. Compile → copy `output_files/*.rbf` to `_Other/PAL_YYYYMMDD.rbf`

## Controls (OSD Define joystick)

| Bit / button | Action |
|--------------|--------|
| D-pad | Move (game) / unused (testpattern) |
| A (Confirm) | Confirm |
| B (Cancel) | Cancel |
| Start (Menu) | Menu |

# MiSTer PAL2

Hybrid MiSTer FPGA core that runs [SDLPAL](https://github.com/sdlpal/sdlpal) (仙剑一 engine) on the HPS, with native FPGA video/audio aimed at NeoGeo-class **320×224** RGBS/HDMI timing. Game content is **320×200**, letterboxed in software.

## License

- **Software:** [GPL-3.0](LICENSE) — see [LICENSE.md](LICENSE.md) and [NOTICE.md](NOTICE.md)
- **Game data:** **not included** (Softstar proprietary). Provide your own legally obtained data.

Upstream SDLPAL is GPL-3.0: you may modify and publish forks under GPL-3.0. A “non-commercial only” license cannot replace GPL for this combined work.

## Layout

| Path | Contents |
|------|----------|
| `src/` | ARM host: native video/audio writer, input, SDLPAL MiSTer glue |
| `fpga/` | Quartus project + RTL (`PAL.sv`, video reader/timing) |
| `sdlpal/` | SDLPAL engine sources (patched for MiSTer) |
| `games/PAL2/` | Deploy stubs (`_handler.sh`, `version.txt`) — **no** game assets |
| `_Other/PAL2.rbf` | Optional prebuilt FPGA bitstream (when present) |

## Build (ARM)

On a Linux/WSL host with `arm-linux-gnueabihf-gcc`:

```bash
cd MiSTer_PAL   # or repo root
cmake -B build-arm -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
  -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ \
  -DCMAKE_EXE_LINKER_FLAGS="-static"
cmake --build build-arm -j"$(nproc)"
```

Copy the `PAL` binary to `/media/fat/games/PAL2/PAL` on the MiSTer.

## FPGA

Open `fpga/` in Quartus (Cyclone V / MiSTer). Output RBF goes to `/media/fat/_Other/PAL2.rbf`.

## Runtime (MiSTer)

1. Install game data under `/media/fat/games/PAL2/Games/` (retail files; lowercase names on Linux).
2. Optional OGG music under `Games/ogg/` (`NN.ogg`) if using the OGG music path.
3. Load **PAL2** from the MiSTer menu; handler starts `./PAL -game …`.

## Credits

- SDLPAL development team / Wei Mingzhi — engine ([GPL-3.0](https://github.com/sdlpal/sdlpal))
- MiSTer hybrid-core patterns (PICO-8 / related GPL cores)
- Softstar — original game (data not redistributed here)

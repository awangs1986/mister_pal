# MiSTer PAL2

Hybrid MiSTer FPGA core that runs [SDLPAL](https://github.com/sdlpal/sdlpal) (*Chinese Paladin* / 仙剑奇侠传 I) on the HPS, with native FPGA video/audio aimed at NeoGeo-class **320×224** RGBS/HDMI. In-game content is **320×200**, letterboxed in software (small black bars top/bottom).

本项目是跑在 MiSTer 上的仙剑一（SDLPAL）混合核：FPGA 负责原生视频/音频时序，游戏逻辑由 HPS 上的 ARM 运行。

---

## Why this project exists / 为什么做这个版本

### 中文

家里有一台 CRT，一直想在**隔行电视**上玩经典仙剑。

官方/常见的 MiSTer DOS 方案一进游戏就花屏，没法正常用。与其干瞪眼，不如动手改；既然要改，索性把更完整的 **98 柔情版**搬上 MiSTer。

FPGA 擅长模拟主机，却不像通用 CPU 那样直接跑这类 PC 游戏。受 [MiSTer PICO-8](https://github.com/MiSTerOrganize/MiSTer_PICO-8) 一类混合核启发，本项目调用 MiSTer 上那颗**本来只负责菜单/OSD 的 ARM（HPS）**来跑引擎——对，整机大半性能都被「锁」在 FPGA 侧，真正跑仙剑的是菜单用的那颗小芯片。

原作分辨率是 **320×200**，落到 320×224 时上下会有一点黑边；在 CRT 上反而意外好看。

音乐方面直接用了 Steam 创意工坊里这套接近完美的 Roland SC 音源包：  
[PAL SC Soundtracks](https://steamcommunity.com/sharedfiles/filedetails/?id=2433259482)

在我自己摸过的隔行方案里，这大概是目前最流畅、观感最好的仙剑一：比 DOS 版优化更多、迷宫更友好，也明显好过土星版。

### English

I have a CRT at home and always wanted to play the classic *Chinese Paladin* on an **interlaced** set.

The usual MiSTer DOS approach corrupted the picture as soon as the game started. Rather than give up, I set out to fix it—and once I was in that deep, it made sense to target the fuller **Win98 “柔情”** edition instead of DOS.

FPGAs are great at console-style timing, but they are not a general-purpose PC. Inspired by hybrid cores such as MiSTer PICO-8, this project runs the engine on MiSTer’s **HPS ARM**—the same small CPU that normally only drives the menu/OSD. Most of the board’s muscle stays on the FPGA side; the game itself runs on that “menu chip.”

The original art is **320×200**, so on a 320×224 NeoGeo-class raster you get thin letterbox bars. On a CRT the result looks surprisingly good.

For BGM this build uses the Roland SC workshop pack:  
[PAL SC Soundtracks](https://steamcommunity.com/sharedfiles/filedetails/?id=2433259482)

Among interlaced setups I’ve tried, this is the smoothest and most complete *Paladin I* experience I know—much more polished than DOS, kinder mazes, and clearly ahead of the Saturn port.

---

## License / 许可

- **Software / 软件:** [GPL-3.0](LICENSE) — see [LICENSE.md](LICENSE.md) and [NOTICE.md](NOTICE.md)
- **Game data / 游戏资源:** **not included / 不随仓库提供** (Softstar proprietary). You must supply your own legally obtained files.

Upstream SDLPAL is GPL-3.0: forks and modified branches are allowed under GPL-3.0. A blanket “non-commercial only” license cannot replace GPL for this combined work.

---

## Game data setup (required) / 正版资源怎么放（必做）

This repo ships **code only**. Copy retail data onto the MiSTer SD card yourself. Do **not** commit game files to GitHub.

本仓库只含代码。请自行把正版数据拷到 MiSTer 的 SD 卡。**不要**把素材提交进 GitHub。

### 1) Buy / obtain the game · 获取正版游戏

Recommended: purchase [*Sword and Fairy* / 仙剑奇侠传](https://store.steampowered.com/app/1546570/) on Steam (AppID **1546570**), which includes the editions SDLPAL can use.

Also acceptable: any **legally owned** 98 柔情 / Win95 data set that SDLPAL supports.

### 2) Copy game data → MiSTer · 游戏本体放到哪里

On the MiSTer SD card, place the game files here:

```text
/media/fat/games/PAL2/Games/
```

Typical contents (names may vary by edition; on Linux/MiSTer prefer **lowercase**):

| Examples | Role |
|----------|------|
| `abc.mkf`, `map.mkf`, `sss.mkf`, `pat.mkf`, … | Core data packs |
| `mgo.mkf`, `rgm.mkf`, `ball.mkf`, … | Graphics / objects |
| `fbp.mkf`, `fire.mkf`, `rng.mkf`, … | More assets |
| `word.dat` / fonts as required by your edition | Text |
| Optional: `Musics/`, AVI, etc. | Depends on edition; this build forces **OGG** BGM (below) |

**Local clone of this repo (optional, for packing before copy):**

```text
games/PAL2/Games/     ← same layout; gitignored, never push
```

Handler launches:

```bash
./PAL -nativevideo -game /media/fat/games/PAL2/Games
```

### 3) Steam Workshop OGG music (Roland SC) · 创意工坊 OGG 音乐

Workshop item used by this project:

- [PAL SC Soundtracks](https://steamcommunity.com/sharedfiles/filedetails/?id=2433259482) (`id=2433259482`)

#### Where Steam stores the download · Steam 本机路径

After you **own the game**, **subscribe** to the workshop item, and let Steam finish downloading:

```text
<Steam>\steamapps\workshop\content\1546570\2433259482\
```

Default Steam root on Windows is often:

```text
C:\Program Files (x86)\Steam\steamapps\workshop\content\1546570\2433259482\
```

If Steam is on another drive/library, replace `<Steam>` with that library’s `steamapps` parent.  
若订阅后文件夹仍为空：在 Steam 里打开游戏一次，或在创意工坊页面确认已订阅并完成下载。

Inside that folder you should see numbered OGG files (and possibly an `ogg` subfolder—open it if present).

#### Where to put them for MiSTer PAL2 · 拷到本项目/MiSTer 的哪里

This core expects **2-digit** names under an `ogg` directory next to the game data:

```text
/media/fat/games/PAL2/Games/ogg/01.ogg
/media/fat/games/PAL2/Games/ogg/02.ogg
...
/media/fat/games/PAL2/Games/ogg/86.ogg   # count depends on the pack
```

Same layout if you stage files in the git tree before copying to the SD card:

```text
games/PAL2/Games/ogg/NN.ogg     ← gitignored
```

**Copy steps / 操作步骤:**

1. Subscribe to the workshop item while logged into Steam with the purchased game.
2. Open `...\workshop\content\1546570\2433259482\`.
3. Copy every `*.ogg` into `/media/fat/games/PAL2/Games/ogg/`.
4. Rename if needed so names match **`NN.ogg`** (`01.ogg` … `09.ogg`, not `1.ogg` or `001.ogg`).  
   - If the pack already uses `01.ogg` style, copy as-is.  
   - If it uses `001.ogg` / `1.ogg`, rename to two digits.
5. Workshop notes sometimes ask to duplicate missing tracks (e.g. copy `06`→`07`); follow the workshop description if a track is silent.

MiSTer build **forces `MUSIC_OGG`** — without `Games/ogg/NN.ogg`, BGM will be silent / empty.

### 4) Deploy binaries · 部署程序（简述）

| File | MiSTer path |
|------|-------------|
| FPGA bitstream | `/media/fat/_Other/PAL2.rbf` |
| ARM binary | `/media/fat/games/PAL2/PAL` |
| Handler | `/media/fat/games/PAL2/_handler.sh` |
| Game + OGG | `/media/fat/games/PAL2/Games/` (+ `ogg/`) |

Then load **PAL2** from the MiSTer menu.

---

## Layout / 目录

| Path | Contents |
|------|----------|
| `src/` | ARM host: native video/audio, input, SDLPAL glue |
| `fpga/` | Quartus project + RTL |
| `sdlpal/` | SDLPAL engine (MiSTer patches) |
| `games/PAL2/` | Deploy stubs only — **no** retail assets in git |
| `_Other/PAL2.rbf` | Optional prebuilt bitstream |

---

## Build (ARM)

On Linux/WSL with `arm-linux-gnueabihf-gcc`:

```bash
cmake -B build-arm -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
  -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ \
  -DCMAKE_EXE_LINKER_FLAGS="-static"
cmake --build build-arm -j"$(nproc)"
```

Copy the resulting `PAL` binary to `/media/fat/games/PAL2/PAL`.

## FPGA

Open `fpga/` in Quartus (Cyclone V / MiSTer). Output RBF → `/media/fat/_Other/PAL2.rbf`.

---

## Credits

- SDLPAL / Wei Mingzhi & contributors — engine ([GPL-3.0](https://github.com/sdlpal/sdlpal))
- [PAL SC Soundtracks](https://steamcommunity.com/sharedfiles/filedetails/?id=2433259482) — Roland SC OGG pack (obtain via Steam Workshop; not redistributed here)
- MiSTer hybrid-core patterns (PICO-8 / related GPL cores)
- Softstar — original game (data not redistributed here)

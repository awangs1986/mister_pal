# MiSTer PAL95

### Lineage & thanks / 项目渊源与致谢

**中文：**  
本仓库是在上游开源工作上的二次开发，不是从零凭空写出来的：

- **游戏引擎：** 基于 [SDLPAL](https://github.com/sdlpal/sdlpal)（仙剑一开源引擎）修改与移植。  
- **MiSTer 混合核框架：** 借鉴并改编自 [MiSTer PICO-8](https://github.com/MiSTerOrganize/MiSTer_PICO-8) 的 FPGA + HPS（ARM）架构思路。

再次衷心感谢 **SDLPAL** 全体作者与贡献者（含 Wei Mingzhi 等），以及 **MiSTer PICO-8 / MiSTer 社区**相关作者——没有你们的工作，就不会有这个项目。

**English：**  
This repository is a derivative / follow-on project, not a from-scratch codebase:

- **Game engine:** based on [SDLPAL](https://github.com/sdlpal/sdlpal) (open-source *Chinese Paladin I* engine), modified and ported here.  
- **MiSTer hybrid-core scaffolding:** adapted from the FPGA + HPS (ARM) approach of [MiSTer PICO-8](https://github.com/MiSTerOrganize/MiSTer_PICO-8).

Once again, sincere thanks to the **SDLPAL** authors and contributors (including Wei Mingzhi and the SDLPAL team), and to the **MiSTer PICO-8 / MiSTer community** authors—this project would not exist without your work.

---

Hybrid MiSTer FPGA core that runs [SDLPAL](https://github.com/sdlpal/sdlpal) (*Chinese Paladin* / 仙剑奇侠传 I) on the HPS, with native FPGA video/audio aimed at NeoGeo-class **320×224** RGBS/HDMI. In-game content is **320×200**, letterboxed in software (small black bars top/bottom).

本项目是跑在 MiSTer 上的仙剑一（SDLPAL）混合核：FPGA 负责原生视频/音频时序，游戏逻辑由 HPS 上的 ARM 运行。

### A casual hobby project / 兴趣随手做的

**中文：**  
这是兴趣驱动、随手做着玩的项目，不保证你那边一定能跑通。遇到 bug 欢迎提 issue 提醒我改；我懒得打正式 Release。如果你对编译/部署一窍不通，最省事的办法是：把本仓库链接丢给 AI，再把你 MiSTer 的 IP 和 root 密码也给它，让它帮你全自动编好、拷好、配好。

**English：**  
This is a casual hobby project. I do **not** promise it will work on every setup. Bugs are welcome—open an issue and nudge me to fix them. I am too lazy to ship polished Releases. If you do not want to deal with builds at all, the easiest path is: give an AI this repo URL plus your MiSTer’s IP and root password, and let it compile, deploy, and configure everything for you.

> Security note / 安全提醒：把 root 密码交给 AI 有风险，仅在你信任的本地/私密环境使用；用完可改密码。

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

**中文：** 本仓库只含代码，不含任何游戏素材。请自行把正版数据拷到 MiSTer 的 SD 卡。**不要**把素材提交进 GitHub。

**English：** This repo ships **code only**. Copy retail data onto the MiSTer SD card yourself. Do **not** commit game files to GitHub.

### 1) Buy / obtain the game · 获取正版游戏

**中文：**  
推荐在 Steam 购买 [*Sword and Fairy* / 仙剑奇侠传](https://store.steampowered.com/app/1546570/)（AppID **1546570**），内含 SDLPAL 可用的版本。  
也可以使用你合法拥有的 **98 柔情 / Win95** 数据包（SDLPAL 支持的即可）。

**English：**  
Recommended: purchase [*Sword and Fairy*](https://store.steampowered.com/app/1546570/) on Steam (AppID **1546570**).  
Also fine: any **legally owned** Win98 “柔情” / Win95 data set that SDLPAL supports.

### 2) Copy game data → MiSTer · 游戏本体放到哪里

**中文：** 在 MiSTer 的 SD 卡上，把游戏数据文件放到：

**English：** On the MiSTer SD card, place the game files here:

```text
/media/fat/games/PAL2/Games/
```

**中文：** 常见文件（具体文件名因版本略有差异；在 Linux/MiSTer 上建议全部用**小写**文件名）：

**English：** Typical contents (names may vary; on Linux/MiSTer prefer **lowercase**):

| 示例 / Examples | 作用 / Role |
|-----------------|-------------|
| `abc.mkf`, `map.mkf`, `sss.mkf`, `pat.mkf`, … | 核心数据包 / Core data |
| `mgo.mkf`, `rgm.mkf`, `ball.mkf`, … | 图形与物件 / Graphics |
| `fbp.mkf`, `fire.mkf`, `rng.mkf`, … | 其它素材 / More assets |
| `word.dat` 等字库 | 文字 / Text |
| 可选 `Musics/`、AVI 等 | 视版本而定；本核背景音乐强制走 **OGG**（见下一步） |

**中文：** 若你先在电脑上的本仓库目录里整理再拷到 SD 卡，可用同样结构（该路径已被 gitignore，切勿 push）：

**English：** Optional local staging in this repo (gitignored—never push):

```text
games/PAL2/Games/
```

**中文：** 启动脚本实际会这样启动游戏：

**English：** The handler launches:

```bash
./PAL -nativevideo -game /media/fat/games/PAL2/Games
```

### 3) Steam Workshop OGG music (Roland SC) · 创意工坊 OGG 音乐

**中文：** 本项目背景音乐使用创意工坊音源包：

**English：** BGM pack used by this project:

- [PAL SC Soundtracks](https://steamcommunity.com/sharedfiles/filedetails/?id=2433259482) (`id=2433259482`)

#### Where Steam stores the download · Steam 本机路径

**中文：**  
先确保你已购买游戏，在 Steam 创意工坊**订阅**该物品，并等下载完成。文件一般在：

**English：**  
Own the game, **subscribe** to the item, wait for Steam to finish downloading:

```text
<Steam>\steamapps\workshop\content\1546570\2433259482\
```

**中文：** Windows 默认 Steam 路径常见为：

**English：** Default Windows Steam root is often:

```text
C:\Program Files (x86)\Steam\steamapps\workshop\content\1546570\2433259482\
```

**中文：**  
- 若 Steam 装在别的盘/游戏库，把 `<Steam>` 换成对应库的根目录再进 `steamapps\...`。  
- 订阅后文件夹仍是空的：进一次 Steam 里的仙剑，或回创意工坊确认已订阅且下载完成。  
- 打开该目录后应能看到编号的 `.ogg`；若还有一层 `ogg` 子文件夹，进子文件夹再拷。

**English：**  
- If Steam lives on another drive/library, replace `<Steam>` accordingly.  
- Empty folder after subscribe: launch the game once, or re-check the workshop download.  
- You should see numbered `.ogg` files (open an `ogg` subfolder if present).

#### Where to put them for MiSTer PAL2 · 拷到哪里

**中文：** 本核要求音乐放在游戏数据旁的 `ogg` 目录，文件名为**两位数字**（`01.ogg`，不是 `1.ogg` 或 `001.ogg`）：

**English：** This core expects **2-digit** names under `ogg` next to the game data:

```text
/media/fat/games/PAL2/Games/ogg/01.ogg
/media/fat/games/PAL2/Games/ogg/02.ogg
...
/media/fat/games/PAL2/Games/ogg/86.ogg   # 数量以音源包为准 / count depends on the pack
```

**中文：** 在本仓库里暂存时同样结构（gitignore）：

**English：** Same layout for local staging (gitignored):

```text
games/PAL2/Games/ogg/NN.ogg
```

**Copy steps / 操作步骤：**

1. **中文：** 用已购买仙剑的 Steam 账号订阅上述创意工坊物品。  
   **English：** Subscribe while logged into Steam with the purchased game.
2. **中文：** 打开 `...\workshop\content\1546570\2433259482\`。  
   **English：** Open that workshop folder.
3. **中文：** 把里面所有 `*.ogg` 复制到 `/media/fat/games/PAL2/Games/ogg/`。  
   **English：** Copy every `*.ogg` into `/media/fat/games/PAL2/Games/ogg/`.
4. **中文：** 如有需要，改名为 **`NN.ogg`**（`01.ogg`…`09.ogg`）。已是两位数字则可直接拷；若是 `001.ogg` / `1.ogg` 请改成两位。  
   **English：** Rename to **`NN.ogg`** if needed (`01`…`09`, not `1` or `001`).
5. **中文：** 创意工坊说明里若要求补缺曲（例如复制 `06`→`07`），按说明做；某首没声时再检查是否缺号。  
   **English：** Follow workshop notes for duplicated/missing tracks if a cue is silent.

**中文：** 本核强制 `MUSIC_OGG`。没有 `Games/ogg/NN.ogg` 就没有背景音乐。

**English：** This build **forces `MUSIC_OGG`**. Without `Games/ogg/NN.ogg`, BGM stays silent.

### 4) Deploy binaries · 部署程序（简述）

| 文件 / File | MiSTer 路径 / Path |
|-------------|-------------------|
| FPGA 比特流 / bitstream | `/media/fat/_Other/PAL2.rbf` |
| ARM 程序 / binary | `/media/fat/games/PAL2/PAL` |
| 启动脚本 / handler | `/media/fat/games/PAL2/_handler.sh` |
| 游戏数据 + OGG | `/media/fat/games/PAL2/Games/`（含 `ogg/`） |

**中文：** 拷好后在 MiSTer 菜单加载 **PAL2** 即可。

**English：** Then load **PAL2** from the MiSTer menu.

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

# MiSTer PAL95

### Lineage & thanks / 项目渊源与致谢

**中文：**  
本仓库是在上游开源工作上的二次开发，不是从零凭空写出来的：

- **MiSTer 平台：** 运行于 [MiSTer FPGA](https://github.com/MiSTer-devel/Main_MiSTer)（DE10-Nano + 开源框架）。没有 MiSTer，就没有这个核的容身之处。  
- **游戏引擎：** 基于 [SDLPAL](https://github.com/sdlpal/sdlpal)（仙剑一开源引擎）修改与移植。  
- **MiSTer 混合核框架：** 借鉴并改编自 [MiSTer PICO-8](https://github.com/MiSTerOrganize/MiSTer_PICO-8) 的 FPGA + HPS（ARM）架构思路。  
- **CRT / 时序关键参数：** 参考了 [NeoGeo MiSTer](https://github.com/MiSTer-devel/NeoGeo_MiSTer) 项目的经验（如 320×224 一类与隔行/RGBS 相关的时序与同步做法）。

再次衷心感谢 **MiSTer** 项目（Sorgelig 与全体贡献者）、**SDLPAL** 全体作者与贡献者（含 Wei Mingzhi 等）、**MiSTer PICO-8** 相关作者，以及 **NeoGeo MiSTer** 与整个 MiSTer 社区——没有你们的工作，就不会有这个项目。

**English：**  
This repository is a derivative / follow-on project, not a from-scratch codebase:

- **MiSTer platform:** runs on [MiSTer FPGA](https://github.com/MiSTer-devel/Main_MiSTer) (DE10-Nano + open framework). Without MiSTer, this core has nowhere to live.  
- **Game engine:** based on [SDLPAL](https://github.com/sdlpal/sdlpal) (open-source *Chinese Paladin I* engine), modified and ported here.  
- **MiSTer hybrid-core scaffolding:** adapted from the FPGA + HPS (ARM) approach of [MiSTer PICO-8](https://github.com/MiSTerOrganize/MiSTer_PICO-8).  
- **CRT-critical timing:** drew on experience from [NeoGeo MiSTer](https://github.com/MiSTer-devel/NeoGeo_MiSTer) (e.g. 320×224-class timing and sync practices relevant to interlaced / RGBS CRTs).

Once again, sincere thanks to the **MiSTer** project (Sorgelig and all contributors), the **SDLPAL** authors and contributors (including Wei Mingzhi and the SDLPAL team), the **MiSTer PICO-8** authors, and the **NeoGeo MiSTer** / wider MiSTer community—this project would not exist without your work.

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
| 可选 `Musics/`、AVI、`mus.mkf` 等 | 视版本而定；默认推荐 **OGG**（见下一步），RIX/MIDI 未在 MiSTer 上调通 |

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

**中文：**  
当前默认配置走 `MUSIC_OGG`：按上面放好 `Games/ogg/NN.ogg` 就会有 BGM；缺了这些文件，背景音乐通常是静音的。

若你更想用**原版音源**，也可以改用 **RIX**（如 `mus.mkf`）或 **MIDI**（如 `Musics/*.mid`）——SDLPAL 本身支持这些类型。但我**没有在 MiSTer 上调试过** RIX/MIDI 通路，不保证一次就能成功（理论上可以，需要自己改音乐类型配置并准备对应文件）。

**English：**  
This build is set up for `MUSIC_OGG` by default: place `Games/ogg/NN.ogg` as above for BGM; without those files, music is usually silent.

If you prefer the **original soundtrack path**, you can also try **RIX** (e.g. `mus.mkf`) or **MIDI** (e.g. `Musics/*.mid`)—SDLPAL supports those formats. I have **not debugged** RIX/MIDI on MiSTer, so success is **not guaranteed** (it should work in theory; you would need to switch the music type in config and supply the matching files yourself).

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

## Build help / 编译帮助

**中文：** 下面是自己从源码编 ARM 程序和 FPGA 核的步骤。不会编的话，把本仓库链接 + MiSTer IP/密码丢给 AI 往往更快。  
**English：** Steps to build the ARM binary and FPGA bitstream from source. If that is too much hassle, give an AI this repo URL plus your MiSTer IP/root password.

### Prerequisites / 环境要求

**中文：**
- Linux 或 **WSL2**（推荐 Ubuntu）
- ARM 交叉编译器：`arm-linux-gnueabihf-gcc` / `g++`，以及 `cmake`、`make`
- FPGA：Intel/Altera **Quartus**（Cyclone V / DE10-Nano / MiSTer 常用版本），能打开本仓库 `fpga/` 工程
- 一台已能联网/SSH 的 **MiSTer**（用于拷贝产物；默认 SSH：`root`，密码以你机器为准）

**English：**
- Linux or **WSL2** (Ubuntu recommended)
- ARM cross toolchain: `arm-linux-gnueabihf-gcc` / `g++`, plus `cmake` and `make`
- FPGA: Intel/Altera **Quartus** for Cyclone V / DE10-Nano / MiSTer, able to open `fpga/`
- A reachable **MiSTer** for deploy (SSH `root`; password is yours)

安装交叉编译器示例（Debian/Ubuntu） / toolchain example:

```bash
sudo apt update
sudo apt install -y build-essential cmake git \
  g++-arm-linux-gnueabihf gcc-arm-linux-gnueabihf
```

### 1) Build ARM binary (`PAL`) / 编译 ARM 程序

**中文：** 仓库内已包含 `sdlpal/`。在仓库根目录执行：

**English：** This repo vendors `sdlpal/`. From the repo root:

```bash
cmake -B build-arm -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
  -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ \
  -DCMAKE_C_FLAGS="-mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon" \
  -DCMAKE_CXX_FLAGS="-mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon" \
  -DCMAKE_EXE_LINKER_FLAGS="-static"
cmake --build build-arm -j"$(nproc)"
```

**中文：**  
- 成功后二进制一般在 `build-arm/PAL`（以 CMake 实际输出为准）。  
- **建议静态链接**（上面的 `-static`），避免 MiSTer 上 glibc 版本不够用。  
- 用 `file build-arm/PAL` 应看到 `ARM` / `statically linked` 一类字样。

**English：**  
- Output binary is usually `build-arm/PAL` (check your CMake output).  
- **Static link** (`-static`) is recommended so old MiSTer glibc is not a problem.  
- `file build-arm/PAL` should report ARM / statically linked.

拷到 MiSTer / copy to MiSTer:

```bash
scp build-arm/PAL root@<MISTER_IP>:/media/fat/games/PAL2/PAL
ssh root@<MISTER_IP> "chmod +x /media/fat/games/PAL2/PAL"
```

同时确保有启动脚本 / also ensure the handler exists:

```text
/media/fat/games/PAL2/_handler.sh
```

（可用仓库里的 `games/PAL2/_handler.sh` 上传。）

### 2) Build FPGA bitstream (`PAL2.rbf`) / 编译 FPGA

**中文：**
1. 用 Quartus 打开 `fpga/` 下的工程（如 `PAL2.qsf` / 工程主文件，以目录内实际名为准）。  
2. 目标器件按 **MiSTer / DE10-Nano（Cyclone V）** 配置，不要改成别的板子。  
3. 执行 **Analysis & Synthesis → Fitter → Assembler**（或一键 Compile）。  
4. 在 `fpga/output_files/`（或工程设定的输出目录）找到生成的 **`.rbf`**。  
5. 拷到 MiSTer：

**English：**
1. Open the Quartus project under `fpga/` (e.g. `PAL2.qsf` — use the files actually in the tree).  
2. Target **MiSTer / DE10-Nano (Cyclone V)**; do not retarget another board.  
3. Run full compile (Synthesis → Fitter → Assembler).  
4. Pick up the **`.rbf`** from `fpga/output_files/` (or your project output dir).  
5. Copy to MiSTer:

```bash
scp <your>.rbf root@<MISTER_IP>:/media/fat/_Other/PAL2.rbf
```

**中文：** 仓库若已带 `_Other/PAL2.rbf`，可跳过 FPGA 编译，直接用现成比特流试跑。  
**English：** If `_Other/PAL2.rbf` is already in the repo, you can skip Quartus and try that bitstream first.

### 3) Game data (not built) / 游戏数据（不是编译出来的）

**中文：** 编译**不会**生成 Softstar 素材。正版数据与 OGG 的放置见上文「正版资源怎么放」。  
**English：** Building does **not** produce Softstar assets. See **Game data setup** above for retail files and Workshop OGG.

### 4) Run on MiSTer / 在 MiSTer 上运行

**中文：**
1. 在 MiSTer 菜单加载 **`_Other/PAL2.rbf`**（或你命名的 PAL2 核）。  
2. Handler 应自动启动 `/media/fat/games/PAL2/PAL`。  
3. 日志常见位置：`/media/fat/logs/PAL2/PAL.log`（可 `grep DIAG`）。  
4. 若黑屏/无声：先确认 `Games/` 与 `Games/ogg/`，再确认只有**一个** `PAL` 进程。

**English：**
1. Load **`_Other/PAL2.rbf`** from the MiSTer menu.  
2. The handler should start `/media/fat/games/PAL2/PAL`.  
3. Logs are often at `/media/fat/logs/PAL2/PAL.log` (`grep DIAG`).  
4. Black screen / silence: check `Games/` + `Games/ogg/`, and that only **one** `PAL` process is running.

### 5) Common pitfalls / 常见坑

| 现象 / Symptom | 可能原因 / Likely cause |
|----------------|-------------------------|
| `GLIBC_2.xx not found` | 未静态链接，或用了主机本机 gcc 而非 armhf 交叉编译 |
| 有核无游戏 / core loads, no game | 缺 `_handler.sh`，或 `PAL` 不在 `/media/fat/games/PAL2/` |
| 无音乐 / no BGM | 缺 `Games/ogg/NN.ogg`（两位数字文件名） |
| 花屏/不同步 / bad sync on CRT | 用了错误的 `.rbf`，或未走本核原生视频路径 |
| 双进程音频炸裂 / audio noise | 两个 `./PAL` 同时写音频环 — `killall PAL` 后只留一个 |

---

## Credits

- [MiSTer FPGA](https://github.com/MiSTer-devel/Main_MiSTer) — Sorgelig & contributors (platform)
- SDLPAL / Wei Mingzhi & contributors — engine ([GPL-3.0](https://github.com/sdlpal/sdlpal))
- [PAL SC Soundtracks](https://steamcommunity.com/sharedfiles/filedetails/?id=2433259482) — Roland SC OGG pack (obtain via Steam Workshop; not redistributed here)
- MiSTer hybrid-core patterns ([PICO-8](https://github.com/MiSTerOrganize/MiSTer_PICO-8))
- CRT / video timing experience ([NeoGeo MiSTer](https://github.com/MiSTer-devel/NeoGeo_MiSTer))
- Softstar — original game (data not redistributed here)

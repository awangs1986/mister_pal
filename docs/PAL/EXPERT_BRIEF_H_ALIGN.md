# PAL2 / NeoGeo hybrid — expert brief (H align / left-shift)

**Date:** 2026-07-20  
**Host:** MiSTer `192.168.5.2` — core `_Other/PAL2.rbf` (`PAL_VIDEO_NEOGEO=1`)  
**ARM:** `./PAL -nativevideo -testpattern` → DDR3 320×224 RGB565 color bars @ `0x3A000000+0x100`  
**Display:** RGBS CRT ~15 kHz, scandoubler off  
**Principle:** fill NeoGeo **native 320×224** (not 200+letterbox; not invented porches)

---

## Resolution (2026-07-20 CRT, after diagnostic RBF)

**Half-width / left-shift theories are REJECTED.**

1. Grid shows col **0** (left, overscan-cropped) through col **19** (right, overscan-cropped) → full **320** reaches CRT.
2. Old testpattern’s last band was **black (40 px = 1/8)**; colored region ends at cell **~17.5** — looked like “bars on the left.”
3. No magenta underrun → DDR/FIFO OK for full lines.
4. Do **not** reintroduce `pxcnt<320` clamps or 304-as-timing; keep full CHBL as NeoGeo 320 mode.
5. Next ARM pattern: bright last band + white edges at **x=0** and **x=319** (FPGA timing unchanged).

---

## What already worked (A/B)

| Experiment | Result |
|------------|--------|
| `BYPASS_DDR_RGB=1` — bars from LSPC `PIXELC`/`RASTERC` only, no DDR | Grid **straight**, no top skew → **sync path can be OK** |
| Official NeoGeo_MiSTer on same CRT/cable | OK → **hardware OK** |
| DDR path + overlay | Overlay (from `pxcnt`/`vcount`) straight; DDR used to shear → **reader/FIFO** |
| `pix_taken` vs `hpos` + drain-to-320 | **Top skew gone** |
| Clamp active to `pxcnt < 320` + cleaner `HBlank \| (pxcnt>=320)` | Intended to kill left-align-in-wide-CHBL; **user still sees left bias / incomplete right** |

---

## Architecture (PAL2)

```
ARM 320×224 RGB565 → DDR3
        ↓
pal_video_reader (clk_sys DDR + clk_vid pixel pop, FIFO 256×64b, 80 words/line)
        ↓ r/g/b
neo_video_timing (clocks_sync → lspc2_clk → videosync → DAC → video_cleaner → video_mixer LINE_LENGTH=320)
        ↓
RGBS CRT
```

Copied Neo pieces (byte-oriented ports from NeoGeo_MiSTer):  
`fpga/rtl/neo_sync/{clocks,lspc2_clk,videosync,resetp}.v` + DAC/VSync rebuild pattern from `neogeo.sv`.

---

## Key code (current)

### 1) Overlay + pxcnt (`neo_video_timing.sv`)

- Overlay cells: 16×16, 20×14 over 320×224; corners force digit `0` (inset toward center).
- `pxcnt` increments on `ce_pix_r`; reset when `HBlank[1:0]==2'b10` (same as NeoGeo DAC).
- Digits black, grid white; composited over DDR when `!CHBL && nBNKB && pxcnt < 320`.

### 2) Active clamp (suspected incomplete fix)

```systemverilog
wire neo_hactive = (pxcnt < 10'd320) || (HBlank[1:0] == 2'b10);
wire hblank_pre  = CHBL | ~neo_hactive;
assign de = ~(hblank_pre | ~nBNKB);  // effectively

// video_cleaner:
.HBlank(HBlank[0] | (pxcnt >= 10'd320))
```

Reader `de` uses this clamped window. Goal: NeoGeo `CHBL` wider than 320 must not leave 320 px of DDR on the left of a wider active.

### 3) Official NeoGeo reference (`neogeo.sv` ~2021–2030)

```systemverilog
PAL_RAM_REG <= (... && ((pxcnt >= 7 && pxcnt < 311) || ~status[16])) ? PAL_RAM_DATA : 16'h8000;
// status[16]: OSD Width 320px vs 304px
HBlank <= (HBlank<<1) | CHBL;
if(HBlank[1:0] == 2'b10) pxcnt <= 0;
// cleaner still gets HBlank[0] from CHBL delay — NOT pxcnt-clamped in stock core
video_mixer #(.LINE_LENGTH(320), ...)
```

Stock core does **not** clamp cleaner HBlank to 320; 304 mode only zeros palette outside `[7,311)`.

### 4) Reader pixel path (`pal_video_reader.sv`)

- `LINE_BURST=80`, `V_ACTIVE=224`, `scale_1to1=1`.
- Separate `hpos` (display) vs `pix_taken` (FIFO pixels consumed).
- Underrun: black, do not pop FIFO; at line end drain until `pix_taken==320`.
- Never wipe `pixel_word_valid` mid-word (that previously sheared).
- `new_frame` at VBlank start; 2-line preload then `frame_ready`.

### 5) ARM

- `mister_main.cpp`: `fill_colorbar` **320×224** full frame (NeoGeo native).
- Writer map: buf0 `+0x100`, buf1 `+0x24000`, 143360 bytes/frame.

---

## Hypotheses still open (for expert)

1. **`pxcnt` is not a linear 0..319 content index** the way we use it — it is reset on delayed CHBL edge and may not equal “NeoGeo visible X”; clamping mixer to `pxcnt<320` may be wrong vs stock (stock passes full CHBL-derived HBlank).
2. **CE / HBlank phase:** reader consumes on `de` from clamped window; DAC latches on `CLK_EN_6MB`; overlay uses same `pxcnt` — residual H phase could crop left “0” and never show cells 10–19 (“90” as last visible).
3. **`video_mixer` + analog path:** `LINE_LENGTH=320` vs actual CE_PIXEL burst length; possible half-line or scaler interaction on RGBS (scandoubler off).
4. **Visible “90”:** if overlay top digits are `[col_ones][row_ones]`, “90” ⇒ cell **(9,0)** — only **10 columns** visible ⇒ ~160 px. That is a strong clue (half of 320), not random overscan.
5. Should hybrid **emit for full CHBL** like stock 320 mode (palette entire active), and only use Neo’s `[7,311)` when emulating 304 — instead of homemade `pxcnt<320` clamp?

---

## Files to review

| Path | Role |
|------|------|
| `fpga/rtl/neo_video_timing.sv` | LSPC wrap, overlay, pxcnt, DE clamp, cleaner/mixer |
| `fpga/rtl/pal_video_reader.sv` | DDR FIFO, pix_taken lock |
| `fpga/rtl/pal_video_top.sv` | Neo vs MD wiring |
| `fpga/rtl/neo_sync/*.v` | Copied Neo sync |
| `fpga/PAL.sv` | Neo audio muted; PLL/video mux |
| `src/mister_main.cpp` | 320×224 bars |
| `refs/NeoGeo_MiSTer/neogeo.sv` | Reference DAC / Width 304\|320 |

**Latest RBF:** `_Other/PAL2.rbf` (~2,518,048 bytes, build with hactive clamp).

---

## Questions for expert

1. Given stock NeoGeo passes **unclamped** `HBlank[0]` from CHBL into `video_cleaner`, what is the correct way for a **320×224 DDR framebuffer** to map into that timing so analog RGBS shows full width (not left half)?
2. Does visible overlay ending at cell **(9,0)** (“90”) indicate **CE_PIXEL / mixer eating half the line**, or **pxcnt/DE only opening ~160 clocks**?
3. Should `de`/reader follow **CHBL** (full), **`pxcnt∈[0,320)`**, or Neo **`pxcnt∈[7,311)`** (304) for native bring-up?
4. Any known pitfall using `video_mixer(.LINE_LENGTH(320))` with LSPC `CLK_EN_6MB` when the source is external DDR rather than PAL RAM?

---

## Ask / constraint from owner

- Do **not** invent custom line rates; stay CRT-safe ~15.7 kHz Neo/SFC/MD family.
- Color bars and content must **fill NeoGeo native 320×224**.
- Prefer 1:1 with `NeoGeo_MiSTer` over more homemade clamps if those fight the stock blanking model.

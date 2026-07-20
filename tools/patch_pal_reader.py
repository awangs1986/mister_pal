#!/usr/bin/env python3
from pathlib import Path

p = Path(__file__).resolve().parents[1] / "fpga/rtl/pal_video_reader.sv"
t = p.read_text(encoding="utf-8")
t = t.replace("module pico8_video_reader", "module pal_video_reader")
t = t.replace("from pico8_video_timing", "from pal_video_timing")

repls = [
    (
        "localparam [28:0] BUF1_ADDR   = 29'h07401020;  // 0x3A008100 >> 3",
        "localparam [28:0] BUF1_ADDR   = 29'h07403EA0;  // 0x3A01F500 >> 3",
    ),
    (
        "localparam [7:0]  LINE_BURST  = 8'd32;         // 128px * 2B / 8 = 32 beats",
        "localparam [7:0]  LINE_BURST  = 8'd80;         // 320px * 2B / 8 = 80 beats",
    ),
    (
        "localparam [28:0] LINE_STRIDE = 29'd32;        // 32 qword addresses per source line",
        "localparam [28:0] LINE_STRIDE = 29'd80;        // 80 qword addresses per source line",
    ),
    (
        "localparam [8:0]  V_ACTIVE    = 9'd224;        // NES exact (128 source -> 1.75x via Bresenham 4/7)",
        "localparam [8:0]  V_ACTIVE    = 9'd240;        // 320x240 letterbox output",
    ),
    (
        "localparam [6:0]  SRC_LINES   = 7'd128;        // source lines in DDR3",
        "localparam [7:0]  SRC_LINES   = 8'd200;        // source lines in DDR3",
    ),
    (
        "localparam [28:0] CART_DATA_ADDR = 29'h07404000;  // 0x3A020000 >> 3 (past video buffers)",
        "localparam [28:0] CART_DATA_ADDR = 29'h0740A000;  // 0x3A050000 >> 3",
    ),
    (
        "localparam [28:0] AUD_RING_ADDR  = 29'h07402040;  // 0x3A010200 >> 3",
        "localparam [28:0] AUD_RING_ADDR  = 29'h07408000;  // 0x3A040000 >> 3",
    ),
    (
        "localparam [8:0] V_BORDER_1TO1 = 9'd48;   // (224-128)/2",
        "localparam [8:0] V_BORDER_1TO1 = 9'd20;   // (240-200)/2 letterbox",
    ),
    (
        "| ((display_line >= V_BORDER_1TO1) && (display_line < (V_BORDER_1TO1 + 9'd128)));",
        "| ((display_line >= V_BORDER_1TO1) && (display_line < (V_BORDER_1TO1 + 9'd200)));",
    ),
    (
        "localparam [8:0] H_BORDER_1TO1 = 9'd64;   // (256-128)/2",
        "localparam [8:0] H_BORDER_1TO1 = 9'd0;    // 320 content fills 320 active",
    ),
    (
        "wire in_content_col = (hpos >= H_BORDER_1TO1) && (hpos < (H_BORDER_1TO1 + 9'd128));",
        "wire in_content_col = (hpos >= H_BORDER_1TO1) && (hpos < (H_BORDER_1TO1 + 9'd320));",
    ),
    (
        "wire [6:0] source_line = scale_1to1 ? dl_src_1to1[6:0] : safe_src_line;",
        "wire [7:0] source_line = scale_1to1 ? dl_src_1to1[7:0] : {1'b0, safe_src_line};",
    ),
]

for a, b in repls:
    if a not in t:
        print("MISS:", a[:70])
    else:
        t = t.replace(a, b)
        print("OK:", a[:60])

p.write_text(t, encoding="utf-8")
print("wrote", p)

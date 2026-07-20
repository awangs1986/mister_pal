//============================================================================
//
//  NeoGeo LSPC video timing for PAL2 — copied from NeoGeo_MiSTer
//
//  Byte-identical RTL copies:
//    rtl/neo_sync/clocks.v     ← refs/.../rtl/io/clocks.v
//    rtl/neo_sync/lspc2_clk.v  ← refs/.../rtl/video/lspc2_clk.v
//    rtl/neo_sync/videosync.v  ← refs/.../rtl/video/videosync.v
//    rtl/neo_sync/resetp.v     ← refs/.../rtl/io/resetp.v  (official resetp_sync)
//
//  Align overlay: OVERLAY_ALIGN draws linear pxcnt/vcount grid+digits on RGB.
//  Corner cells force digit 0. BYPASS_DDR_RGB=1 = internal bars (A/B sync).
//
//============================================================================

module neo_video_timing (
    input  wire        clk,
    input  wire        reset,        // active-high (MiSTer RESET)

    input  wire  [7:0] r_in,
    input  wire  [7:0] g_in,
    input  wire  [7:0] b_in,
    input  wire        video_en,
    input  wire        reader_underrun, // unused in game path (was diagnostic paint)

    inout  wire [21:0] gamma_bus,

    output wire        ce_pix,
    output wire        ce_pix_out,
    output wire  [7:0] vga_r,
    output wire  [7:0] vga_g,
    output wire  [7:0] vga_b,
    output wire        vga_hs,
    output wire        vga_vs,
    output wire        vga_de,
    output wire        hblank,
    output wire        vblank,
    output wire        de,
    output wire        new_line,
    output wire        new_frame,
    output wire  [8:0] vcount,
    output wire        bypass_ddr   // 1 = internal bars; force VGA active in top
);

// 0 = DDR reader RGB (normal hybrid). Overlay was bring-up only — keep off for game.
localparam BYPASS_DDR_RGB = 1'b0;
// 1 = white grid + black cell numbers over DDR (linear pxcnt/vcount; corners = 0)
localparam OVERLAY_ALIGN  = 1'b0;
assign bypass_ddr = BYPASS_DDR_RGB;
wire _unused_reader_underrun = reader_underrun;

// --- neogeo.sv:196-200 (exact) ---
reg CLK_EN_24M_N, CLK_EN_24M_P;
always @(posedge clk) begin
    CLK_EN_24M_N <= ~CLK_EN_24M_N;
    CLK_EN_24M_P <= CLK_EN_24M_N;
end

// Official resetp_sync (lspc2_a2.v:709). Port nRESET is active-low.
// neogeo passes nRESET into LSPC.RESET which feeds resetp.
wire nRESET  = ~reset;
wire nRESETP;

resetp_sync rstp (
    .CLK(clk),
    .CLK_EN_24_N(CLK_EN_24M_N),
    .nRESET(nRESET),
    .nRESETP(nRESETP)
);

wire CLK_EN_6MB;
wire LSPC_3M, LSPC_1_5M;
wire LSPC_EN_1_5M_P, LSPC_EN_1_5M_N;

clocks_sync clocks (
    .CLK(clk),
    .CLK_EN_24M_P(CLK_EN_24M_P),
    .CLK_EN_24M_N(CLK_EN_24M_N),
    .nRESETP(nRESETP),
    .CLK_24M(),
    .CLK_12M(),
    .CLK_68KCLK(),
    .CLK_68KCLKB(),
    .CLK_EN_68K_P(),
    .CLK_EN_68K_N(),
    .CLK_6MB(),
    .CLK_1HB(),
    .CLK_EN_12M(),
    .CLK_EN_12M_N(),
    .CLK_EN_6MB(CLK_EN_6MB),
    .CLK_EN_1HB()
);

lspc2_clk_sync lspc_clk (
    .CLK(clk),
    .CLK_EN_24M_P(CLK_EN_24M_P),
    .CLK_EN_24M_N(CLK_EN_24M_N),
    .nRESETP(nRESETP),
    .CLK_24MB(),
    .LSPC_12M(),
    .LSPC_8M(),
    .LSPC_6M(),
    .LSPC_4M(),
    .LSPC_3M(LSPC_3M),
    .LSPC_1_5M(LSPC_1_5M),
    .Q53_CO(),
    .LSPC_EN_12M_P(),
    .LSPC_EN_12M_N(),
    .LSPC_EN_6M_P(),
    .LSPC_EN_6M_N(),
    .LSPC_EN_3M(),
    .LSPC_EN_1_5M_P(LSPC_EN_1_5M_P),
    .LSPC_EN_1_5M_N(LSPC_EN_1_5M_N),
    .LSPC_EN_4M_P(),
    .LSPC_EN_4M_N()
);

wire       HSync;
wire       CHBL;
wire       nBNKB;
wire [8:0] PIXELC;
wire [8:0] RASTERC;
// Official: VMODE = status[3]. PAL2 fixed NTSC (0) — match AES default.
// Confirm official NeoGeo OSD is also NTSC when A/B testing.
wire       VMODE = 1'b0;

videosync_sync vsync_gen (
    .CLK(clk),
    .CLK_EN_24M_P(CLK_EN_24M_P),
    .CLK_EN_24M_N(CLK_EN_24M_N),
    .LSPC_3M(LSPC_3M),
    .LSPC_1_5M(LSPC_1_5M),
    .LSPCE_EN_1_5M_P(LSPC_EN_1_5M_P),
    .LSPCE_EN_1_5M_N(LSPC_EN_1_5M_N),
    .nRESETP(nRESETP),
    .VMODE(VMODE),
    .PIXELC(PIXELC),
    .RASTERC(RASTERC),
    .HSYNC(HSync),
    .VSYNC(),
    .BNK(),
    .BNKB(nBNKB),
    .CHBL(CHBL),
    .R15_QD(),
    .FLIP(),
    .nFLIP(),
    .P50_CO()
);

// --- Align overlay: linear active coords (pxcnt/vcount), NOT raw PIXELC ---
// 320×224 → 20×14 cells of 16×16. Four corner cells force digit 0 for CRT align.
localparam [4:0] CELL_COLS = 5'd20; // 320/16
localparam [4:0] CELL_ROWS = 5'd14; // 224/16

reg        ce_pix_r;
reg  [2:0] HBlank;
reg  [7:0] R8, G8, B8;
reg        new_line_tog;
reg  [8:0] vcount_r;
reg        nBNKB_d;
reg  [9:0] pxcnt;

wire [4:0] cell_col = pxcnt[8:4];
wire [4:0] cell_row = vcount_r[8:4];
wire [3:0] cell_fx  = pxcnt[3:0];
wire [3:0] cell_fy  = vcount_r[3:0];
wire       at_tl = (cell_col == 5'd0) && (cell_row == 5'd0);
wire       at_tr = (cell_col == CELL_COLS - 5'd1) && (cell_row == 5'd0);
wire       at_bl = (cell_col == 5'd0) && (cell_row == CELL_ROWS - 5'd1);
wire       at_br = (cell_col == CELL_COLS - 5'd1) && (cell_row == CELL_ROWS - 5'd1);
wire       at_corner = at_tl | at_tr | at_bl | at_br;

// 5×7 font row (MSB = left). Digits 0-9.
function automatic [4:0] font5x7_row;
    input [3:0] digit;
    input [2:0] y; // 0..6
    begin
        case (digit)
            4'd0: case (y)
                0: font5x7_row = 5'b01110; 1: font5x7_row = 5'b10001;
                2: font5x7_row = 5'b10011; 3: font5x7_row = 5'b10101;
                4: font5x7_row = 5'b11001; 5: font5x7_row = 5'b10001;
                default: font5x7_row = 5'b01110;
            endcase
            4'd1: case (y)
                0: font5x7_row = 5'b00100; 1: font5x7_row = 5'b01100;
                2: font5x7_row = 5'b00100; 3: font5x7_row = 5'b00100;
                4: font5x7_row = 5'b00100; 5: font5x7_row = 5'b00100;
                default: font5x7_row = 5'b01110;
            endcase
            4'd2: case (y)
                0: font5x7_row = 5'b01110; 1: font5x7_row = 5'b10001;
                2: font5x7_row = 5'b00001; 3: font5x7_row = 5'b00010;
                4: font5x7_row = 5'b00100; 5: font5x7_row = 5'b01000;
                default: font5x7_row = 5'b11111;
            endcase
            4'd3: case (y)
                0: font5x7_row = 5'b11110; 1: font5x7_row = 5'b00001;
                2: font5x7_row = 5'b00001; 3: font5x7_row = 5'b01110;
                4: font5x7_row = 5'b00001; 5: font5x7_row = 5'b00001;
                default: font5x7_row = 5'b11110;
            endcase
            4'd4: case (y)
                0: font5x7_row = 5'b00010; 1: font5x7_row = 5'b00110;
                2: font5x7_row = 5'b01010; 3: font5x7_row = 5'b10010;
                4: font5x7_row = 5'b11111; 5: font5x7_row = 5'b00010;
                default: font5x7_row = 5'b00010;
            endcase
            4'd5: case (y)
                0: font5x7_row = 5'b11111; 1: font5x7_row = 5'b10000;
                2: font5x7_row = 5'b11110; 3: font5x7_row = 5'b00001;
                4: font5x7_row = 5'b00001; 5: font5x7_row = 5'b10001;
                default: font5x7_row = 5'b01110;
            endcase
            4'd6: case (y)
                0: font5x7_row = 5'b00110; 1: font5x7_row = 5'b01000;
                2: font5x7_row = 5'b10000; 3: font5x7_row = 5'b11110;
                4: font5x7_row = 5'b10001; 5: font5x7_row = 5'b10001;
                default: font5x7_row = 5'b01110;
            endcase
            4'd7: case (y)
                0: font5x7_row = 5'b11111; 1: font5x7_row = 5'b00001;
                2: font5x7_row = 5'b00010; 3: font5x7_row = 5'b00100;
                4: font5x7_row = 5'b01000; 5: font5x7_row = 5'b01000;
                default: font5x7_row = 5'b01000;
            endcase
            4'd8: case (y)
                0: font5x7_row = 5'b01110; 1: font5x7_row = 5'b10001;
                2: font5x7_row = 5'b10001; 3: font5x7_row = 5'b01110;
                4: font5x7_row = 5'b10001; 5: font5x7_row = 5'b10001;
                default: font5x7_row = 5'b01110;
            endcase
            4'd9: case (y)
                0: font5x7_row = 5'b01110; 1: font5x7_row = 5'b10001;
                2: font5x7_row = 5'b10001; 3: font5x7_row = 5'b01111;
                4: font5x7_row = 5'b00001; 5: font5x7_row = 5'b00010;
                default: font5x7_row = 5'b01100;
            endcase
            default: font5x7_row = 5'b00000;
        endcase
    end
endfunction

function automatic digit_on;
    input [3:0] digit;
    input [3:0] fx, fy; // position inside 16×16 cell
    input [3:0] ox, oy; // top-left of glyph in cell
    reg [2:0] gx, gy;
    reg [4:0] rowbits;
    begin
        digit_on = 1'b0;
        if (fx >= ox && fx < ox + 4'd5 && fy >= oy && fy < oy + 4'd7) begin
            gx = fx - ox;
            gy = fy - oy;
            rowbits = font5x7_row(digit, gy);
            digit_on = rowbits[4 - gx];
        end
    end
endfunction

reg        ink;
reg        grid_line;
always @(*) begin
    grid_line = (cell_fx == 4'd0) || (cell_fy == 4'd0);
    ink = 1'b0;

    if (at_corner) begin
        // One large-ish 0 per corner, inset toward screen center so CRT
        // overscan (which bisects the corner cell) still shows a full digit.
        if (at_tl)
            ink = ink | digit_on(4'd0, cell_fx, cell_fy, 4'd9, 4'd7);
        else if (at_tr)
            ink = ink | digit_on(4'd0, cell_fx, cell_fy, 4'd2, 4'd7);
        else if (at_bl)
            ink = ink | digit_on(4'd0, cell_fx, cell_fy, 4'd9, 4'd2);
        else // at_br
            ink = ink | digit_on(4'd0, cell_fx, cell_fy, 4'd2, 4'd2);
    end else begin
        // Unambiguous coordinates: top = two-digit column, bottom = row.
        // The old interleaved layout made column 09 and the upper half of
        // column 19 both look like "90", which defeated the alignment test.
        ink = ink | digit_on(cell_col / 10, cell_fx, cell_fy, 4'd2, 4'd2);
        ink = ink | digit_on(cell_col % 10, cell_fx, cell_fy, 4'd9, 4'd2);
        ink = ink | digit_on(cell_row / 10, cell_fx, cell_fy, 4'd2, 4'd9);
        ink = ink | digit_on(cell_row % 10, cell_fx, cell_fy, 4'd9, 4'd9);
    end
end

// --- neogeo.sv:2014-2031 DAC latches ---
always @(posedge clk) begin
    ce_pix_r <= 1'b0;

    if (CLK_EN_6MB) begin
        ce_pix_r <= 1'b1;
        if (BYPASS_DDR_RGB) begin
            if (!CHBL && nBNKB) begin
                if (OVERLAY_ALIGN && ink) begin
                    R8 <= 8'h00; G8 <= 8'h00; B8 <= 8'h00; // black digits
                end else if (OVERLAY_ALIGN && grid_line) begin
                    R8 <= 8'hFF; G8 <= 8'hFF; B8 <= 8'hFF; // white grid
                end else begin
                    R8 <= 8'h40;
                    G8 <= 8'h40;
                    B8 <= 8'h40;
                end
            end else begin
                R8 <= 8'd0;
                G8 <= 8'd0;
                B8 <= 8'd0;
            end
        end else if (video_en) begin
            // Native 320 mode follows the official NeoGeo CHBL window.
            if (CHBL || ~nBNKB) begin
                R8 <= 8'd0; G8 <= 8'd0; B8 <= 8'd0;
            end else if (OVERLAY_ALIGN && ink) begin
                R8 <= 8'h00; G8 <= 8'h00; B8 <= 8'h00; // black digits
            end else if (OVERLAY_ALIGN && grid_line) begin
                R8 <= 8'hFF; G8 <= 8'hFF; B8 <= 8'hFF; // white grid
            end else begin
                // Clean game path — no diagnostic seam/crop/underrun paints.
                R8 <= r_in;
                G8 <= g_in;
                B8 <= b_in;
            end
        end else begin
            R8 <= 8'd0;
            G8 <= 8'd0;
            B8 <= 8'd0;
        end
    end

    if (ce_pix_r) begin
        HBlank  <= (HBlank << 1) | CHBL;
        nBNKB_d <= nBNKB;

        pxcnt <= pxcnt + 10'd1;
        if (HBlank[1:0] == 2'b10)
            pxcnt <= 10'd0;

        if (~nBNKB_d & nBNKB)
            vcount_r <= 9'd0;

        if (HBlank[1:0] == 2'b01) begin
            new_line_tog <= ~new_line_tog;
            if (nBNKB)
                vcount_r <= vcount_r + 9'd1;
        end
    end

    if (!nRESETP) begin
        HBlank       <= 3'd0;
        pxcnt        <= 10'd0;
        new_line_tog <= 1'b0;
        vcount_r     <= 9'd0;
        nBNKB_d      <= 1'b0;
    end
end

assign ce_pix   = ce_pix_r;
assign new_line = new_line_tog;
assign vcount   = vcount_r;

// One horizontal truth source. Full CHBL is NeoGeo's native 320px mode;
// the official [7,311) test is a 304px content crop, not a timing window.
assign hblank = CHBL;
assign vblank = ~nBNKB;
assign de     = ~(CHBL | ~nBNKB);

// --- neogeo.sv:2034-2060 VSync rebuild ---
reg VSync;
reg RFSH;
reg new_frame_r;
always @(posedge clk) begin
    reg       old_hs;
    reg       old_vbl;
    reg [2:0] vbl;
    reg [7:0] vblcnt, vspos, rfsh_cnt;

    new_frame_r <= 1'b0;

    if (ce_pix_r) begin
        old_hs <= HSync;
        if (~old_hs & HSync) begin
            old_vbl <= nBNKB;

            if (~nBNKB) vblcnt <= vblcnt + 1'd1;
            if (old_vbl & ~nBNKB) begin
                vblcnt      <= 0;
                new_frame_r <= 1'b1; // VBlank start (hybrid DDR prep)
            end
            if (~old_vbl & nBNKB) begin
                vspos    <= (vblcnt >> 1) - 8'd7;
                rfsh_cnt <= vblcnt - 2'd2;
            end

            {VSync, vbl} <= {vbl, 1'b0};
            if (vblcnt == vspos) {VSync, vbl} <= '1;
        end

        RFSH <= (vblcnt < rfsh_cnt);
    end
end

assign new_frame = new_frame_r;

wire [7:0] r_c, g_c, b_c;
wire       hs_c, vs_c;
wire       hblank_c, vblank_c;

video_cleaner video_cleaner (
    .clk_vid(clk),
    .ce_pix(ce_pix_r),
    .R(R8),
    .G(G8),
    .B(B8),
    .HSync(HSync),
    .VSync(VSync),
    // Official NeoGeo path uses the delayed CHBL sample directly.
    .HBlank(HBlank[0]),
    .VBlank(~nBNKB),
    .DE_in(1'b0),
    .interlace(1'b0),
    .f1(1'b0),
    .VGA_R(r_c),
    .VGA_G(g_c),
    .VGA_B(b_c),
    .VGA_VS(vs_c),
    .VGA_HS(hs_c),
    .HBlank_out(hblank_c),
    .VBlank_out(vblank_c),
    .DE_out()
);

video_mixer #(.LINE_LENGTH(320), .HALF_DEPTH(0), .GAMMA(1)) video_mixer
(
    .CLK_VIDEO(clk),
    .CE_PIXEL(ce_pix_out),
    .ce_pix(ce_pix_r),
    .scandoubler(1'b0),
    .hq2x(1'b0),
    .gamma_bus(gamma_bus),
    .R(r_c),
    .G(g_c),
    .B(b_c),
    .HSync(hs_c),
    .VSync(vs_c),
    .HBlank(hblank_c),
    .VBlank(vblank_c),
    .HDMI_FREEZE(1'b0),
    .freeze_sync(),
    .VGA_R(vga_r),
    .VGA_G(vga_g),
    .VGA_B(vga_b),
    .VGA_VS(vga_vs),
    .VGA_HS(vga_hs),
    .VGA_DE(vga_de)
);

endmodule

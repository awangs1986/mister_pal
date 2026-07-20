//============================================================================
//  PAL Native Video Top — timing + DDR3 reader
//  Adapted from MiSTerOrganize/MiSTer_PICO-8 — GPL-3.0
//
//  PAL_VIDEO_NEOGEO: NeoGeo LSPC sync chain (neo_video_timing) — no porch tables
//  else:            MegaDrive H40 porch timing (pal_video_timing)
//============================================================================

module pal_video_top (
    input  wire        clk_sys,
    input  wire        clk_vid,
    input  wire        ce_pix,       // MD path only; ignored when PAL_VIDEO_NEOGEO
    output wire        ce_pix_out,   // CE_PIXEL source (NeoGeo: CLK_EN_6MB path)

    input  wire        reset,

    input  wire        ddr_busy,
    output wire  [7:0] ddr_burstcnt,
    output wire [28:0] ddr_addr,
    input  wire [63:0] ddr_dout,
    input  wire        ddr_dout_ready,
    output wire        ddr_rd,
    output wire [63:0] ddr_din,
    output wire  [7:0] ddr_be,
    output wire        ddr_we,

    output wire  [7:0] vga_r,
    output wire  [7:0] vga_g,
    output wire  [7:0] vga_b,
    output wire        vga_hs,
    output wire        vga_vs,
    output wire        vga_de,

    input  wire        enable,
    input  wire        scale_1to1,
    output wire        active,
    output wire        vsync_out,

    inout  wire [21:0] gamma_bus,   // NeoGeo path → video_mixer GAMMA=1

    input  wire  [2:0] h_offset,
    input  wire  [2:0] v_offset,

    input  wire        clk_audio,
    output wire [15:0] audio_l,
    output wire [15:0] audio_r,

    input  wire [31:0] joystick_0,
    input  wire [31:0] joystick_1,
    input  wire [31:0] joystick_2,
    input  wire [31:0] joystick_3,
    input  wire [15:0] joystick_l_analog_0,

    input  wire        ioctl_download,
    input  wire        ioctl_wr,
    input  wire [26:0] ioctl_addr,
    input  wire  [7:0] ioctl_dout,
    output wire        ioctl_wait,

    input  wire        ss_save,
    input  wire        ss_load,
    input  wire  [1:0] ss_slot
);

wire        tim_hsync, tim_vsync;
wire        tim_hblank, tim_vblank;
wire        tim_de;
wire [8:0]  tim_vcount;
wire        tim_new_frame, tim_new_line;
wire        tim_ce_pix;

wire [7:0]  reader_r, reader_g, reader_b;
wire        reader_frame_ready;
wire        reader_video_underrun;
wire [15:0] reader_audio_l, reader_audio_r;

`ifdef PAL_VIDEO_NEOGEO
// ---------------------------------------------------------------------------
// NeoGeo path: clocks → lspc2_clk → videosync → DAC → cleaner → video_mixer(320)
// ---------------------------------------------------------------------------
wire [7:0] neo_r, neo_g, neo_b;
wire       neo_hs, neo_vs, neo_de;
wire       neo_ce_mix;
wire       neo_bypass_ddr;

neo_video_timing neo_timing (
    .clk(clk_vid),
    .reset(reset),
    .r_in(reader_r),
    .g_in(reader_g),
    .b_in(reader_b),
    .video_en(reader_frame_ready),
    .reader_underrun(reader_video_underrun),
    .gamma_bus(gamma_bus),
    .ce_pix(tim_ce_pix),
    .ce_pix_out(neo_ce_mix),
    .vga_r(neo_r),
    .vga_g(neo_g),
    .vga_b(neo_b),
    .vga_hs(neo_hs),
    .vga_vs(neo_vs),
    .vga_de(neo_de),
    .hblank(tim_hblank),
    .vblank(tim_vblank),
    .de(tim_de),
    .new_line(tim_new_line),
    .new_frame(tim_new_frame),
    .vcount(tim_vcount),
    .bypass_ddr(neo_bypass_ddr)
);

assign ce_pix_out = neo_ce_mix; // video_mixer CE_PIXEL (NeoGeo exact)
assign vga_r      = neo_r;
assign vga_g      = neo_g;
assign vga_b      = neo_b;
assign vga_hs     = neo_hs;
assign vga_vs     = neo_vs;
assign vga_de     = neo_de;
assign vsync_out  = neo_vs;
`else
// ---------------------------------------------------------------------------
// MegaDrive H40 porch timing
// ---------------------------------------------------------------------------
wire signed [4:0] h_adj = (h_offset == 3'd0) ?  5'sd0 :
                          (h_offset == 3'd1) ?  5'sd4 :
                          (h_offset == 3'd2) ?  5'sd8 :
                          (h_offset == 3'd3) ?  5'sd12 :
                          (h_offset == 3'd4) ? -5'sd12 :
                          (h_offset == 3'd5) ? -5'sd8 :
                                               -5'sd4;
wire signed [3:0] v_adj = (v_offset == 3'd0) ?  4'sd0 :
                          (v_offset == 3'd1) ?  4'sd1 :
                          (v_offset == 3'd2) ?  4'sd2 :
                          (v_offset == 3'd3) ?  4'sd3 :
                          (v_offset == 3'd4) ? -4'sd3 :
                          (v_offset == 3'd5) ? -4'sd2 :
                                               -4'sd1;

wire [9:0] tim_hcount;

pal_video_timing timing (
    .clk(clk_vid), .ce_pix(ce_pix), .reset(reset),
    .h_adj(h_adj), .v_adj(v_adj),
    .hsync(tim_hsync), .vsync(tim_vsync),
    .hblank(tim_hblank), .vblank(tim_vblank),
    .de(tim_de), .hcount(tim_hcount), .vcount(tim_vcount),
    .new_frame(tim_new_frame), .new_line(tim_new_line)
);

assign tim_ce_pix = ce_pix;
assign ce_pix_out = ce_pix;
assign vga_r      = reader_frame_ready ? reader_r : 8'd0;
assign vga_g      = reader_frame_ready ? reader_g : 8'd0;
assign vga_b      = reader_frame_ready ? reader_b : 8'd0;
assign vga_hs     = tim_hsync;
assign vga_vs     = tim_vsync;
assign vga_de     = tim_de;
assign vsync_out  = tim_vsync;
`endif

pal_video_reader reader (
    .ddr_clk(clk_sys),
    .ddr_busy(ddr_busy),
    .ddr_burstcnt(ddr_burstcnt),
    .ddr_addr(ddr_addr),
    .ddr_dout(ddr_dout),
    .ddr_dout_ready(ddr_dout_ready),
    .ddr_rd(ddr_rd),
    .ddr_din(ddr_din),
    .ddr_be(ddr_be),
    .ddr_we(ddr_we),
    .clk_vid(clk_vid),
    .ce_pix(tim_ce_pix),
    .reset(reset),
    .de(tim_de),
    .hblank(tim_hblank),
    .vblank(tim_vblank),
    .new_frame(tim_new_frame),
    .new_line(tim_new_line),
    .vcount(tim_vcount),
    .r_out(reader_r),
    .g_out(reader_g),
    .b_out(reader_b),
    .clk_audio(clk_audio),
    .audio_l_out(reader_audio_l),
    .audio_r_out(reader_audio_r),
    .enable(enable),
    .scale_1to1(1'b1),
    .frame_ready(reader_frame_ready),
    .video_underrun(reader_video_underrun),
    .joystick_0(joystick_0),
    .joystick_1(joystick_1),
    .joystick_2(joystick_2),
    .joystick_3(joystick_3),
    .joystick_l_analog_0(joystick_l_analog_0),
    .ioctl_download(ioctl_download),
    .ioctl_wr(ioctl_wr),
    .ioctl_addr(ioctl_addr),
    .ioctl_dout(ioctl_dout),
    .ioctl_wait(ioctl_wait),
    .ss_save(ss_save),
    .ss_load(ss_load),
    .ss_slot(ss_slot)
);

`ifdef PAL_VIDEO_NEOGEO
// Internal Neo bars: force active so PAL.sv does not black the VGA mux
assign active = enable & (neo_bypass_ddr | reader_frame_ready);
`else
assign active = enable & reader_frame_ready;
`endif
assign audio_l = reader_audio_l;
assign audio_r = reader_audio_r;

endmodule

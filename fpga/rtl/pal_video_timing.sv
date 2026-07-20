//============================================================================
//
//  PAL Native Video Timing — MegaDrive H40 NTSC only
//
//  PAL.rbf: H_TOTAL=427, pix≈6.712 MHz, V_TOTAL=262, 320x224 active.
//  PAL2 (NeoGeo) uses neo_video_timing.sv — do not invent NeoGeo porches here.
//
//============================================================================

module pal_video_timing (
    input  wire        clk,
    input  wire        ce_pix,
    input  wire        reset,

    input  wire signed [4:0] h_adj,
    input  wire signed [3:0] v_adj,

    output reg         hsync,
    output reg         vsync,
    output reg         hblank,
    output reg         vblank,
    output reg         de,
    output reg  [9:0]  hcount,
    output reg  [8:0]  vcount,
    output reg         new_frame,
    output reg         new_line
);

// MegaDrive_MiSTer H40 NTSC: CLK_VIDEO=107.386 MHz, CE÷16≈6.712 MHz
// f_H ≈ 6.712e6/427 ≈ 15.72 kHz; V_TOTAL=262
localparam H_ACTIVE = 320;
localparam H_FP     = 15;
localparam H_SYNC   = 21;
localparam H_BP     = 71;
localparam H_TOTAL  = 427;

localparam V_ACTIVE = 224;
localparam V_FP     = 12;
localparam V_SYNC   = 4;
localparam V_BP     = 22;
localparam V_TOTAL  = 262;

wire [9:0] h_sync_start = H_ACTIVE + H_FP + {{5{h_adj[4]}}, h_adj};
wire [9:0] h_sync_end   = h_sync_start + H_SYNC;
wire [8:0] v_sync_start = V_ACTIVE + V_FP + {{5{v_adj[3]}}, v_adj};
wire [8:0] v_sync_end   = v_sync_start + V_SYNC;

always @(posedge clk) begin
    if (reset) begin
        hcount    <= 10'd0;
        vcount    <= 9'd0;
        hsync     <= 1'b1;
        vsync     <= 1'b1;
        hblank    <= 1'b0;
        vblank    <= 1'b0;
        de        <= 1'b1;
        new_frame <= 1'b0;
        new_line  <= 1'b0;
    end
    else if (ce_pix) begin
        new_frame <= 1'b0;
        new_line  <= 1'b0;

        if (hcount == H_TOTAL - 1) begin
            hcount <= 10'd0;
            if (vcount == V_TOTAL - 1)
                vcount <= 9'd0;
            else
                vcount <= vcount + 9'd1;
        end
        else begin
            hcount <= hcount + 10'd1;
        end

        if (hcount == H_ACTIVE - 1)
            hblank <= 1'b1;
        else if (hcount == H_TOTAL - 1)
            hblank <= 1'b0;

        if (hcount == h_sync_start - 1)
            hsync <= 1'b0;
        else if (hcount == h_sync_end - 1)
            hsync <= 1'b1;

        if (hcount == H_TOTAL - 1) begin
            if (vcount == V_ACTIVE - 1)
                vblank <= 1'b1;
            else if (vcount == V_TOTAL - 1)
                vblank <= 1'b0;
        end

        if (hcount == H_TOTAL - 1) begin
            if (vcount == v_sync_start - 1)
                vsync <= 1'b0;
            else if (vcount == v_sync_end - 1)
                vsync <= 1'b1;
        end

        if (hcount == H_ACTIVE - 1)
            new_line <= 1'b1;

        if (hcount == H_TOTAL - 1 && vcount == V_ACTIVE - 1)
            new_frame <= 1'b1;

        de <= ~((hcount >= H_ACTIVE) || (vcount >= V_ACTIVE));
    end
end

endmodule

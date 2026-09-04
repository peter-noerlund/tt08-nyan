`default_nettype none

module top
    (
        input wire ICE_CLK,

        output wire [7:0] ICE_PMOD0,
        inout wire [7:0] ICE_PMOD1,

        output wire LED_R,
        output wire LED_G,
        output wire LED_B
    );

    wire clk;
    wire clk_locked;
    wire rst_n;
    wire [7:0] ui_in;
    wire [7:0] uo_out;
    wire [7:0] uio_in;
    wire [7:0] uio_out;
    wire [7:0] uio_oe;

    assign ui_in = 8'b00000000;
    assign ICE_PMOD0 = uo_out;

    assign rst_n = clk_locked;

    assign LED_R = 1'b1;
    assign LED_G = ~clk_locked;
    assign LED_B = 1'b1;

    SB_IO #(
        .PIN_TYPE(6'b1010_01)
    ) uio_pin[7:0] (
        .PACKAGE_PIN(ICE_PMOD1),
        .OUTPUT_ENABLE(uio_oe),
        .D_OUT_0(uio_out),
        .D_IN_0(uio_in),
    );

    pll pll(
        .clock_in(ICE_CLK),
        .clock_out(clk),
        .locked(clk_locked)
    );

    tt_um_nyan nyan(
        .clk(clk),
        .rst_n(rst_n),
        .ena(1'b1),
        .ui_in(ui_in),
        .uo_out(uo_out),
        .uio_in(uio_in),
        .uio_out(uio_out),
        .uio_oe(uio_oe)
    );
endmodule
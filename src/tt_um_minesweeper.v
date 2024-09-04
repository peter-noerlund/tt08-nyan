// tt_um_vga_example

`default_nettype none

module tt_um_minesweeper
    /* verilator lint_off UNUSED */
    (
        input wire [7:0] ui_in,
        output wire [7:0] uo_out,
        input wire [7:0] uio_in,
        output wire [7:0] uio_out,
        output wire [7:0] uio_oe,
        input wire ena,
        input wire clk,
        input wire rst_n
    );
    /* verilator lint_on UNUSED */

    graphics gfx(.clk(clk), .rst_n(rst_n), .vga_pmod(uo_out));
endmodule

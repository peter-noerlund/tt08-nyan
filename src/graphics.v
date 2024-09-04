`default_nettype none

module graphics
    #(
        parameter VGA_WIDTH = 640,
        parameter VGA_HEIGHT = 480,

        parameter H_FRONT_PORCH = 16,
        parameter H_SYNC_PULSE = 96,
        parameter H_BACK_PORCH = 48,

        parameter V_FRONT_PORCH = 10,
        parameter V_SYNC_PULSE = 2,
        parameter V_BACK_PORCH = 33
    )
    (
        input wire clk,
        input wire rst_n,

        output wire [7:0] vga_pmod
    );

    localparam X_PIXEL_BITS = $clog2(VGA_WIDTH + H_FRONT_PORCH + H_SYNC_PULSE + H_BACK_PORCH);
    localparam Y_PIXEL_BITS = $clog2(VGA_HEIGHT + V_FRONT_PORCH + V_SYNC_PULSE + V_BACK_PORCH);

    localparam NYAN_LEFT = 128;
    localparam NYAN_TOP0 = 128;
    localparam NYAN_TOP1 = 136;
    localparam NYAN_SCALE = 8;

    localparam NYAN_TAIL0_LEFT = NYAN_LEFT;
    localparam NYAN_TAIL0_RIGHT = NYAN_TAIL0_LEFT + 6 * NYAN_SCALE;
    localparam NYAN_TAIL0_TOP = NYAN_TOP0 + 8 * NYAN_SCALE;
    localparam NYAN_TAIL0_BOTTOM = NYAN_TAIL0_TOP + 6 * NYAN_SCALE;

    localparam NYAN_TAIL1_LEFT = NYAN_LEFT;
    localparam NYAN_TAIL1_RIGHT = NYAN_TAIL1_LEFT + 6 * NYAN_SCALE;
    localparam NYAN_TAIL1_TOP = NYAN_TOP1 + 11 * NYAN_SCALE;
    localparam NYAN_TAIL1_BOTTOM = NYAN_TAIL1_TOP + 5 * NYAN_SCALE;

    localparam NYAN_BASE_WIDTH = 27 * NYAN_SCALE;
    localparam NYAN_BASE_HEIGHT = 17 * NYAN_SCALE;

    localparam NYAN_BASE0_LEFT = NYAN_TAIL0_RIGHT;
    localparam NYAN_BASE0_RIGHT = NYAN_BASE0_LEFT + NYAN_BASE_WIDTH;
    localparam NYAN_BASE0_TOP = NYAN_TOP0;
    localparam NYAN_BASE0_BOTTOM = NYAN_TOP0 + NYAN_BASE_HEIGHT;

    localparam NYAN_BASE1_LEFT = NYAN_TAIL1_RIGHT;
    localparam NYAN_BASE1_RIGHT = NYAN_BASE1_LEFT + NYAN_BASE_WIDTH;
    localparam NYAN_BASE1_TOP = NYAN_TOP1;
    localparam NYAN_BASE1_BOTTOM = NYAN_TOP1 + NYAN_BASE_HEIGHT;

    localparam NYAN_FEET0_LEFT = NYAN_LEFT + 5 * NYAN_SCALE;
    localparam NYAN_FEET0_RIGHT = NYAN_FEET0_LEFT + 25 * NYAN_SCALE;
    localparam NYAN_FEET0_TOP = NYAN_BASE0_BOTTOM;
    localparam NYAN_FEET0_BOTTOM = NYAN_FEET0_TOP + 3 * NYAN_SCALE;

    localparam NYAN_FEET1_LEFT = NYAN_LEFT + 6 * NYAN_SCALE;
    localparam NYAN_FEET1_RIGHT = NYAN_FEET1_LEFT + 24 * NYAN_SCALE;
    localparam NYAN_FEET1_TOP = NYAN_BASE1_BOTTOM;
    localparam NYAN_FEET1_BOTTOM = NYAN_FEET1_TOP + 3 * NYAN_SCALE;

    reg hsync;
    reg vsync;
    reg [1:0] red;
    reg [1:0] green;
    reg [1:0] blue;

    reg [X_PIXEL_BITS - 1 : 0] pixel_x;
    reg [Y_PIXEL_BITS - 1 : 0] pixel_y;

    reg [4:0] frame_counter;

    assign vga_pmod = {hsync, blue[1], green[1], red[1], vsync, blue[0], green[0], red[0]};
 
    `include "img_base.v"
    `include "img_feet0.v"
    `include "img_feet1.v"
    `include "img_tail0.v"
    `include "img_tail1.v"

    always @ (posedge clk) begin
        if (!rst_n) begin
            hsync <= 1'b1;
            vsync <= 1'b1;
        end else begin
            if (pixel_x == VGA_WIDTH + H_FRONT_PORCH) begin
                hsync <= 1'b1;
            end
            if (pixel_x == VGA_WIDTH + H_FRONT_PORCH + H_SYNC_PULSE) begin
                hsync <= 1'b0;
            end

            if (pixel_y == VGA_HEIGHT + V_FRONT_PORCH) begin
                vsync <= 1'b1;
            end
            if (pixel_y == VGA_HEIGHT + V_FRONT_PORCH + V_SYNC_PULSE) begin
                vsync <= 1'b0;
            end

            if (pixel_x == VGA_WIDTH + H_BACK_PORCH + H_SYNC_PULSE + H_BACK_PORCH - 1) begin
                pixel_x <= {X_PIXEL_BITS{1'b0}};
                if (pixel_y == VGA_HEIGHT + V_FRONT_PORCH + V_SYNC_PULSE + V_BACK_PORCH - 1) begin
                    pixel_y <= {Y_PIXEL_BITS{1'b0}};
                    frame_counter <= frame_counter + 5'd1;
                end else begin
                    pixel_y <= pixel_y + 1;
                end
            end else begin
                pixel_x <= pixel_x + 1;
            end

            if (frame_counter[4] && pixel_x >= NYAN_BASE0_LEFT && pixel_x < NYAN_BASE0_RIGHT && pixel_y >= NYAN_BASE0_TOP && pixel_y < NYAN_BASE0_BOTTOM) begin
                {blue, green, red} <= img_base[(pixel_y - NYAN_BASE0_TOP) / NYAN_SCALE][(pixel_x - NYAN_BASE0_LEFT) / NYAN_SCALE * 6 + 5 -: 6];
            end else if (!frame_counter[4] && pixel_x >= NYAN_BASE1_LEFT && pixel_x < NYAN_BASE1_RIGHT && pixel_y >= NYAN_BASE1_TOP && pixel_y < NYAN_BASE1_BOTTOM) begin
                {blue, green, red} <= img_base[(pixel_y - NYAN_BASE1_TOP) / NYAN_SCALE][(pixel_x - NYAN_BASE1_LEFT) / NYAN_SCALE * 6 + 5 -: 6];
            end else if (frame_counter[4] && pixel_x >= NYAN_FEET0_LEFT && pixel_x < NYAN_FEET0_RIGHT && pixel_y >= NYAN_FEET0_TOP && pixel_y < NYAN_FEET0_BOTTOM) begin
                {blue, green, red} <= img_feet0[(pixel_y - NYAN_FEET0_TOP) / NYAN_SCALE][(pixel_x - NYAN_FEET0_LEFT) / NYAN_SCALE * 6 + 5 -: 6];
            end else if (!frame_counter[4] && pixel_x >= NYAN_FEET1_LEFT && pixel_x < NYAN_FEET1_RIGHT && pixel_y >= NYAN_FEET1_TOP && pixel_y < NYAN_FEET1_BOTTOM) begin
                {blue, green, red} <= img_feet1[(pixel_y - NYAN_FEET1_TOP) / NYAN_SCALE][(pixel_x - NYAN_FEET1_LEFT) / NYAN_SCALE * 6 + 5 -: 6];
            end else if (frame_counter[4] && pixel_x >= NYAN_TAIL0_LEFT && pixel_x < NYAN_TAIL0_RIGHT && pixel_y >= NYAN_TAIL0_TOP && pixel_y < NYAN_TAIL0_BOTTOM) begin
                {blue, green, red} <= img_tail0[(pixel_y - NYAN_TAIL0_TOP) / NYAN_SCALE][(pixel_x - NYAN_TAIL0_LEFT) / NYAN_SCALE * 6 + 5 -: 6];
            end else if (!frame_counter[4] && pixel_x >= NYAN_TAIL1_LEFT && pixel_x < NYAN_TAIL1_RIGHT && pixel_y >= NYAN_TAIL1_TOP && pixel_y < NYAN_TAIL1_BOTTOM) begin
                {blue, green, red} <= img_tail1[(pixel_y - NYAN_TAIL1_TOP) / NYAN_SCALE][(pixel_x - NYAN_TAIL1_LEFT) / NYAN_SCALE * 6 + 5 -: 6];
            end else begin
                {red, green, blue} <= 6'b000111;
            end
        end
    end
endmodule

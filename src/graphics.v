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
            pixel_x <= {X_PIXEL_BITS{1'b0}};
            pixel_y <= {Y_PIXEL_BITS{1'b0}};
            hsync <= 1'b1;
            vsync <= 1'b1;
            red <= 2'd0;
            green <= 2'd0;
            blue <= 2'd0;
            frame_counter <= 5'd0;
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

            if (frame_counter[4] &&
                pixel_x >= 188 + 6 * 8 &&
                pixel_x < 188 + 6 * 8 + 27 * 8 &&
                pixel_y >= 160 &&
                pixel_y < 160 + 17 * 8
            ) begin
                {blue, green, red} <= img_base[16 - (pixel_y - 160) / 8][(26 - (pixel_x - 188 - 6 * 8) / 8) * 6 + 5 -: 6];
            end else if (
                !frame_counter[4] &&
                pixel_x >= 188 + 6 * 8 &&
                pixel_x < 188 + 6 * 8 + 27 * 8 &&
                pixel_y >= 165 &&
                pixel_y < 165 + 17 * 8
            ) begin
                {blue, green, red} <= img_base[16 - (pixel_y - 165) / 8][(26 - (pixel_x - 188 - 6 * 8) / 8) * 6 + 5 -: 6];
            end else if (
                frame_counter[4] &&
                pixel_x >= 188 + 5 * 8 &&
                pixel_x < 188 + 5 * 8 + 25 * 8 &&
                pixel_y >= 160 + 17 * 8 &&
                pixel_y < 160 + 17 * 8 + 3 * 8
            ) begin
                {blue, green, red} <= img_feet0[2 - (pixel_y - 160 - 17 * 8) / 8][(24 - (pixel_x - 188 - 5 * 8) / 8) * 6 + 5 -: 6];
            end else if (
                !frame_counter[4] &&
                pixel_x >= 188 + 6 * 8 &&
                pixel_x < 188 + 6 * 8 + 24 * 8 &&
                pixel_y >= 165 + 17 * 8 &&
                pixel_y < 165 + 17 * 8 + 3 * 8
            ) begin
                {blue, green, red} <= img_feet1[2 - (pixel_y - 165 - 17 * 8) / 8][(23 - (pixel_x - 188 - 6 * 8) / 8) * 6 + 5 -: 6];
            end else if (
                frame_counter[4] &&
                pixel_x >= 188 &&
                pixel_x < 188 + 6 * 8 &&
                pixel_y >= 160 + 8 * 8 &&
                pixel_y < 160 + 8 * 8 + 6 * 8
            ) begin
                {blue, green, red} <= img_tail0[5 - (pixel_y - 160 - 8 * 8) / 8][(5 - (pixel_x - 188) / 8) * 6 + 5 -: 6];
            end else if (
                !frame_counter[4] &&
                pixel_x >= 188 &&
                pixel_x < 188 + 6 * 8 &&
                pixel_y >= 160 + 11 * 8 &&
                pixel_y < 160 + 11 * 8 + 5 * 8
            ) begin
                {blue, green, red} <= img_tail1[4 - (pixel_y - 160 - 11 * 8) / 8][(5 - (pixel_x - 188) / 8) * 6 + 5 -: 6];
            end else begin
                {red, green, blue} <= 6'b000111;
            end
        end
    end
endmodule

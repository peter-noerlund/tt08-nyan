`default_nettype none

module graphics
    #(
        parameter H_PIXELS = 640,
        parameter V_PIXELS = 480,

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

    localparam X_PIXEL_BITS = $clog2(H_PIXELS + H_FRONT_PORCH + H_SYNC_PULSE + H_BACK_PORCH);
    localparam Y_PIXEL_BITS = $clog2(V_PIXELS + V_FRONT_PORCH + V_SYNC_PULSE + V_BACK_PORCH);

    localparam NYAN_SCALE_BITS = 3;
    localparam NYAN_SCALE = 2 ** NYAN_SCALE_BITS;
    localparam NYAN_WIDTH = 34 * NYAN_SCALE;
    localparam NYAN_HEIGHT = 22 * NYAN_SCALE;
    localparam NYAN_TOP = 128;
    localparam NYAN_LEFT = 128;
    localparam NYAN_BOTTOM = NYAN_TOP + NYAN_HEIGHT;
    localparam NYAN_RIGHT = NYAN_LEFT + NYAN_WIDTH;

    reg hsync;
    reg vsync;
    reg [1:0] red;
    reg [1:0] green;
    reg [1:0] blue;

    reg [X_PIXEL_BITS - 1 : 0] pixel_x;
    reg [Y_PIXEL_BITS - 1 : 0] pixel_y;

    reg [4:0] frame_counter;

    wire [5:0] bitmap_x;
    wire [4:0] bitmap_y;

    assign vga_pmod = {hsync, blue[0], green[0], red[0], vsync, blue[1], green[1], red[1]};

    assign bitmap_x = 6'((pixel_x - X_PIXEL_BITS'(NYAN_LEFT)) >> NYAN_SCALE_BITS);
    assign bitmap_y = 5'((pixel_y - Y_PIXEL_BITS'(NYAN_TOP)) >> NYAN_SCALE_BITS);
 
    `include "palette.svh"
    `include "frame0.svh"
    `include "frame1.svh"

    always @ (posedge clk) begin
        if (!rst_n) begin
            hsync <= 1'b1;
            vsync <= 1'b1;

            red <= 2'd0;
            green <= 2'd0;
            blue <= 2'd0;
            pixel_x <= {X_PIXEL_BITS{1'b0}};
            pixel_y <= {Y_PIXEL_BITS{1'b0}};
            frame_counter <= 5'd0;
        end else begin
            if (pixel_x == H_PIXELS + H_FRONT_PORCH) begin
                hsync <= 1'b0;
            end
            if (pixel_x == H_PIXELS + H_FRONT_PORCH + H_SYNC_PULSE) begin
                hsync <= 1'b1;
            end

            if (pixel_y == V_PIXELS + V_FRONT_PORCH) begin
                vsync <= 1'b0;
            end
            if (pixel_y == V_PIXELS + V_FRONT_PORCH + V_SYNC_PULSE) begin
                vsync <= 1'b1;
            end

            if (pixel_x >= H_PIXELS + H_FRONT_PORCH + H_SYNC_PULSE + H_BACK_PORCH - 1) begin
                pixel_x <= {X_PIXEL_BITS{1'b0}};
                if (pixel_y >= V_PIXELS + V_FRONT_PORCH + V_SYNC_PULSE + V_BACK_PORCH - 1) begin
                    pixel_y <= {Y_PIXEL_BITS{1'b0}};
                    frame_counter <= frame_counter + 5'd1;
                end else begin
                    pixel_y <= pixel_y + 1;
                end
            end else begin
                pixel_x <= pixel_x + 1;
            end

            if (pixel_x >= NYAN_LEFT && pixel_x < NYAN_RIGHT && pixel_y >= NYAN_TOP && pixel_y < NYAN_BOTTOM) begin
                {red, green, blue} <=
                    palette
                        [
                            frame_counter[4] == 1'b0 ?
                            frame0[bitmap_x[5:1]][bitmap_y] :
                            frame1[bitmap_x[5:1]][bitmap_y]
                        ]
                        [bitmap_x[0]];
            end else if (pixel_x >= 64 && pixel_x < NYAN_LEFT && pixel_y >= NYAN_TOP && pixel_y < NYAN_BOTTOM) begin
                {red, green, blue} <=
                    palette[frame_counter[4] == 1'b1 ? frame0[0][bitmap_y] : frame1[0][bitmap_y]][0];
            end else if (pixel_x < 64 && pixel_y >= NYAN_TOP && pixel_y < NYAN_BOTTOM) begin
                {red, green, blue} <=
                    palette[frame_counter[4] == 1'b0 ? frame0[0][bitmap_y] : frame1[0][bitmap_y]][0];
            end else if (pixel_x < H_PIXELS && pixel_y < V_PIXELS) begin
                {red, green, blue} <= 6'b000111;
            end else begin
                {red, green, blue} <= 6'b000000;
            end
        end
    end
endmodule

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

    localparam X_PIXEL_BITS = $clog2(VGA_WIDTH);
    localparam Y_PIXEL_BITS = $clog2(VGA_HEIGHT);

    reg hsync;
    reg vsync;
    reg [1:0] red;
    reg [1:0] green;
    reg [1:0] blue;

    reg [X_PIXEL_BITS - 1 : 0] pixel_x;
    reg [Y_PIXEL_BITS - 1 : 0] pixel_y;

    wire [15:0] random_value;

    assign vga_pmod = {hsync, blue[1], green[1], red[1], vsync, blue[0], green[0], red[0]};

    xorshift rng(.clk(clk), .rst_n(rst_n), .enable(1), .value(random_value));

    always @ (posedge clk) begin
        if (!rst_n) begin
            pixel_x <= {X_PIXEL_BITS{1'b0}};
            pixel_y <= {Y_PIXEL_BITS{1'b0}};
            hsync <= 1'b1;
            vsync <= 1'b1;
            red <= 2'd0;
            green <= 2'd0;
            blue <= 2'd0;
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
                end else begin
                    pixel_y <= pixel_y + 1;
                end
            end else begin
                pixel_x <= pixel_x + 1;
            end
            
            if (pixel_x < VGA_WIDTH && pixel_y < VGA_HEIGHT) begin
                red <= random_value[1:0];
                green <= random_value[3:2];
                blue <= random_value[5:4];
            end else begin
                red <= 2'd0;
                green <= 2'd0;
                blue <= 2'd0;
            end
        end
    end
endmodule
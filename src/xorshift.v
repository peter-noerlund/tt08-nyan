`default_nettype none

module xorshift
    (
        input wire clk,
        input wire rst_n,

        input wire enable,
        output wire [15:0] value
    );

    reg [15:0] state;
    wire [15:0] state0;
    wire [15:0] state1;
    wire [15:0] state2;

    assign state0 = state ^ (state << 7);
    assign state1 = state0 ^ (state0 >> 9);
    assign state2 = state1 ^ (state1 << 8);

    assign value = state;

    always @ (posedge clk) begin
        if (!rst_n) begin
            state <= 16'h0001;
        end else if (enable) begin
            state <= state2;
        end
    end
endmodule

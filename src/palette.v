reg [5:0] palette[31:0][1:0];
initial begin
    palette[0][0] = 6'b000000;
    palette[0][1] = 6'b000000;
    palette[1][0] = 6'b000000;
    palette[1][1] = 6'b000111;
    palette[2][0] = 6'b000000;
    palette[2][1] = 6'b101010;
    palette[3][0] = 6'b000000;
    palette[3][1] = 6'b111110;
    palette[4][0] = 6'b000111;
    palette[4][1] = 6'b000000;
    palette[5][0] = 6'b000111;
    palette[5][1] = 6'b000111;
    palette[6][0] = 6'b001011;
    palette[6][1] = 6'b000000;
    palette[7][0] = 6'b001011;
    palette[7][1] = 6'b001011;
    palette[8][0] = 6'b001100;
    palette[8][1] = 6'b000000;
    palette[9][0] = 6'b001100;
    palette[9][1] = 6'b001100;
    palette[10][0] = 6'b101010;
    palette[10][1] = 6'b000000;
    palette[11][0] = 6'b101010;
    palette[11][1] = 6'b101010;
    palette[12][0] = 6'b101010;
    palette[12][1] = 6'b111010;
    palette[13][0] = 6'b101011;
    palette[13][1] = 6'b000000;
    palette[14][0] = 6'b101011;
    palette[14][1] = 6'b101011;
    palette[15][0] = 6'b110000;
    palette[15][1] = 6'b000000;
    palette[16][0] = 6'b110000;
    palette[16][1] = 6'b110000;
    palette[17][0] = 6'b110110;
    palette[17][1] = 6'b111011;
    palette[18][0] = 6'b111000;
    palette[18][1] = 6'b000000;
    palette[19][0] = 6'b111000;
    palette[19][1] = 6'b111000;
    palette[20][0] = 6'b111010;
    palette[20][1] = 6'b000000;
    palette[21][0] = 6'b111010;
    palette[21][1] = 6'b111010;
    palette[22][0] = 6'b111011;
    palette[22][1] = 6'b000000;
    palette[23][0] = 6'b111011;
    palette[23][1] = 6'b110110;
    palette[24][0] = 6'b111011;
    palette[24][1] = 6'b111011;
    palette[25][0] = 6'b111011;
    palette[25][1] = 6'b111110;
    palette[26][0] = 6'b111100;
    palette[26][1] = 6'b000000;
    palette[27][0] = 6'b111100;
    palette[27][1] = 6'b111100;
    palette[28][0] = 6'b111110;
    palette[28][1] = 6'b000000;
    palette[29][0] = 6'b111110;
    palette[29][1] = 6'b111011;
    palette[30][0] = 6'b111110;
    palette[30][1] = 6'b111110;
    palette[31][0] = 6'b111111;
    palette[31][1] = 6'b000000;
end

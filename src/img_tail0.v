reg [35 : 0] img_tail0 [5 : 0];
initial begin
    img_tail0[5] = 36'b110100110100110100110100000000000000;
    img_tail0[4] = 36'b110100110100000000000000010101010101;
    img_tail0[3] = 36'b110100000000010101010101010101010101;
    img_tail0[2] = 36'b000000010101010101000000000000000000;
    img_tail0[1] = 36'b000000010101010101000000110100110100;
    img_tail0[0] = 36'b110100000000000000110100110100110100;
end

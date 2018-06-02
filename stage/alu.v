module alu(clk, a, b, c);
input clk;
input a;
input b;
output c;

logic d;

always @(posedge clk) begin: update_c
c <= d;
end

assign d = a | b;
endmodule
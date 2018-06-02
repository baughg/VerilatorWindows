module simple(clk, a, b, c);
input clk;
input a;
input b;
output c;

alu alu_1(.clk(clk),.a(a),.b(b),.c(c));
endmodule
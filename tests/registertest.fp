%
% Test for the register keyword
% 

define accumulator = reg(2,15);	% define an accumulator register
define op1 = input(1,15);		% input operand1
define op2 = input(1,15);		% input operand1

accumulator = accumulator + (op1 + op2);

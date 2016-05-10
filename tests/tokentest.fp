% This is a line spanning comment
%
% Test file for the tokenizer
% 
% Author: Niels Moseley
%

% define input variables as Q(n,m)
% and define a canonical-signed-digit constant.
define a = input(2,13);
define b = input(-3,8);
define c = csd(3.1415927,5);

% do some actual arithmetic
out1 = c*a + b;
%out2 = c >> 1;
%out3 = c << 2;
%out2 = c >>> 3;
%out3 = c <<< 4;

% test the different forms of floats
x = 3.1415927;
y = 3.1415927e0;


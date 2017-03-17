% Simple example for fptool
% Author: Niels Moseley
% Moseley Instruments
%

% define input variables as Q(n,m)
% and define a canonical-signed-digit constant.
define a = input(1,15);
define b = input(1,15);
define c = input(1,15);

% do some actual arithmetic
out = a + 2*b + c;

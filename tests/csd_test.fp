% Canonical Signed Digit test 
% Author: Niels Moseley
% Moseley Instruments
%

% define input variables as Q(n,m)
% and define a canonical-signed-digit constant.
define a = input(1,15);
define b = csd(3.1415927,4);

% do some actual arithmetic
out = a*b;

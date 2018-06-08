%
% 2nd order biquad IIR filter
% example for FPTOOL
% 
% N.A. Moseley, Moseley Instrumnets
% 27-3-2018
%
% Filter is a second-order butterworth filter
% with a relative cutoff factor of 0.5
% w.r.t. the nyquist frequency.
%

define state1 = reg(1,15);      % first filter state
define state2 = reg(1,15);      % second filter state
define iir_in = input(1,15);    % input precision

% define filter coefficients
%
% B = [0.2929    0.5858    0.2929];
% A = [1.0000    0.0000    0.1716];
%
% which is transformed into:
% pre-gain = 0.2929
% B = [1 2 1];
% A = [1 0 0.1716];

define pre_gain = csd(0.2929, 3);

define mul2 = csd(2.0,1);
define A2 = csd(0.1716, 3);

% calculate filter
sum = truncate(pre_gain*iir_in - A2*state2, 1,15);
iir_out = sum + (mul2*state1) + state2;
state2 = state1;
state1 = sum;

# fptool
A compiler for generating fixed-point logic using VHDL.

Assumptions:
* all variables are SIGNED.
* all intermediate results are scaled to avoid overflow.
* you know what you're doing: optimization are not done (yet).

Built-in functions:
* saturate(x,n,m) saturates variable 'x' to fit it into a Q(n,m) variable.
* truncate(x,n,m) removes (or adds) bits to variable 'x' so it becomes Q(n,m).
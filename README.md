# fptool
A compiler for generating fixed-point logic using VHDL.

Assumptions:
* all variables are SIGNED.
* all intermediate results are scaled to avoid overflow.
* all shift operators are of the arithmetic type and don't drop bits.
* division is not supported (for now).
* you know what you're doing: optimization are not done (for now).
* Q(n,m) has 'm' factional bits and 'n' integer bits.
* the number of bits in a Q(n,m) is n+m.
* Q(1,7) has a range of [-1/128 .. 1/127], i.e. it can't represent 1.0 exactly.

Built-in functions:
* saturate(x,n,m) saturates variable 'x' to fit it into a Q(n,m) variable.
* truncate(x,n,m) removes (or adds) bits to variable 'x' so it becomes Q(n,m).

Operators:
* regular arithmetic: '+' '-' '*' 
* shift: '<<', '>>'
* rotate: '<<<', '>>>'
* line comment: '%'

Current project state:
* Lexer is working.
* Parser can check about 80% of the grammar.

License: GPL v2.

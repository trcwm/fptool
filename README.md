# FPTOOL README
## A compiler for generating fixed-point VHDL code.
### Niels A. Moseley

The fixed-point tool (FPTOOL) takes mathematical expressions and fixed-point input variable definitions, and transforms them into VHDL (or verilog in the future). The compiler takes care of the precision/width of each intermediate result to avoid overflows.

The compiler can generate multipliers with Canonical Signed Digit (CSD) constants, leading to area-efficient implementations.

Assumptions:

- all variables are SIGNED.
- all intermediate results are scaled to avoid overflow.
- all shift operators are of the arithmetic type and don't drop bits.
- division is not supported (for now).
- you know what you're doing: optimizations are not done (for now).
- Q(n,m) has 'm' factional bits and 'n' integer bits.
- the number of bits in a Q(n,m) is n+m.
- Q(1,7) has a range of [-1/128 .. 1/127], i.e. it can't represent 1.0 exactly.

Built-in functions [TODO]:

- saturate(x,n,m) saturates variable 'x' to fit it into a Q(n,m) variable.
- truncate(x,n,m) removes (or adds) bits to variable 'x' so it becomes Q(n,m).

Operators:

- regular arithmetic: '+' '-' '*'
- line comment: '%'
- division operator is accepted but code for it will no be generated.

Current project state:

- Lexer is working.
- Parser can check the grammar (Except <<, >>, <<<, >>> and saturate & truncate) and build an abstract syntax tree.
- VHDL code generator is working (except for division operator)
- CSD expansion should be working (requires more testing)
- Don't use it for production unless you test the output thoroughly!!

## Building
Load the project file (.pro) into [QtCreator](https://www.qt.io/ide/), configure the project for your compiler, then select Build->Build All.

## Command line options

- "-o VHDLFILENAME" to generate VHDL source code.
- "-g DOTFILENAME" to generate Graphviz/Dot formatted AST dump.

License: GPL v2.

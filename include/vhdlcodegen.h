/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  VHDL code generator

*/

#ifndef vhdlcodegen_h
#define vhdlcodegen_h

#include <iostream>
#include "ssapass.h"

class VHDLCodeGen : public SSAPass
{

protected:
    virtual void execute(SSAObject &ssa);

    /** generate left hand side of a statement */
    void genLHS(std::ostream &os, operand_t op, uint32_t indent);

    /** generate signals and variables etc. */
    void genProcessHeader(const SSAObject &ssa, std::ostream &os, uint32_t indent);

    /** generate indentation */
    void genIndent(std::ostream &os, uint32_t indent);
};

#endif

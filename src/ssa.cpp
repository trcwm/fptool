/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Single static assignment intermediate representation
                for code generation.

  Author: Niels A. Moseley

*/

#include "ssa.h"


bool SSA::OpAdd::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpSub::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpMul::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpNegate::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpTruncate::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpAssign::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

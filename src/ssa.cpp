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

bool SSA::OpReinterpret::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpPatchBlock::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpExtendLSBs::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpExtendMSBs::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpRemoveLSBs::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

bool SSA::OpRemoveMSBs::accept(SSA::OperationVisitorBase *visitor)
{
    return visitor->visit(this);
}

void SSA::Program::applyPatches()
{
    auto iter = m_statements.begin();
    while(iter != m_statements.end())
    {
        if ((*iter)->isPatchBlock())
        {
            OpPatchBlock *patchBlock = dynamic_cast<OpPatchBlock*>(*iter);
            iter = m_statements.erase(iter);
            m_statements.insert(iter,
                                patchBlock->m_instructions.begin(),
                                patchBlock->m_instructions.end());

            // delete the patch block instruction itself
            if (patchBlock->m_replacedInstruction != NULL)
            {
                delete patchBlock->m_replacedInstruction;
            }
            delete patchBlock;
        }
        else
        {
            iter++;
        }
    }
}

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

bool SSA::OpCSDMul::accept(SSA::OperationVisitorBase *visitor)
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

bool SSA::OpNull::accept(SSA::OperationVisitorBase *visitor)
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


void SSA::OperationSingle::replaceOperand(const SharedOpPtr &op1, SharedOpPtr op2)
{
    if (m_op == op1)
    {
        m_op = op2;
    }
}


void SSA::OperationDual::replaceOperand(const SharedOpPtr &op1, SharedOpPtr op2)
{
    if (m_op1 == op1)
    {
        m_op1 = op2;
    }
    if (m_op2 == op1)
    {
        m_op2 = op2;
    }
}


void SSA::OpPatchBlock::replaceOperand(const SharedOpPtr &op1, SharedOpPtr op2)
{
    for(OperationBase* statement : m_statements)
    {
        statement->replaceOperand(op1, op2);
    }
}


void SSA::Program::applyPatches()
{
    auto iter = m_statements.begin();
    while(iter != m_statements.end())
    {
        OpNull *nullPtr = dynamic_cast<OpNull*>(*iter);
        if ((*iter)->isPatchBlock())
        {
            OpPatchBlock *patchBlock = dynamic_cast<OpPatchBlock*>(*iter);
            iter = m_statements.erase(iter);
            m_statements.insert(iter,
                                patchBlock->m_statements.begin(),
                                patchBlock->m_statements.end());

            // delete the patch block instruction itself
            if (patchBlock->m_replacedInstruction != NULL)
            {
                delete patchBlock->m_replacedInstruction;
            }
            delete patchBlock;
        }
        else if (nullPtr != NULL)
        {
            // we have found a Null operation
            // -> remove it!
            iter = m_statements.erase(iter);
            delete nullPtr;
        }
        else
        {
            iter++;
        }
    }
}

/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Clean the SSA list after the CSDMul pass

  1) remove re-interpret nodes
  2) remove superluous assignment nodes

*/

#include "logging.h"
#include "pass_clean.h"

using namespace SSA;

bool PassClean::execute(Program &ssa)
{
    doLog(LOG_INFO, "----------------------\n");
    doLog(LOG_INFO, "  Running Clean pass\n");
    doLog(LOG_INFO, "----------------------\n");

    PassClean pass(ssa);

    // remove re-interpreted nodes
    for(auto statement : ssa.m_statements)
    {
        if (!statement->accept(&pass))
        {
            return false;
        }
    }

    ssa.applyPatches();


    return true;
}

bool PassClean::visit(const OpAssign *node)
{
    // remove all superfluous assign nodes
    // which are <temp var1> := <temp var2>

    IntermediateOperand* lhs = dynamic_cast<IntermediateOperand*>(node->m_lhs.get());
    IntermediateOperand* op  = dynamic_cast<IntermediateOperand*>(node->m_op.get());

    if ((lhs != NULL) && (op != NULL))
    {
        doLog(LOG_DEBUG, "Removing assignment %s = %s\n",
              node->m_lhs->m_identName.c_str(),
              node->m_op->m_identName.c_str());

        // replace the assigned var with original var
        substituteOperands(node->m_lhs, node->m_op);

        // replace the assignment node with a Null operation
        // to disable it.
        replaceWithNull(node);
    }

    return true;
}

bool PassClean::visit(const OpReinterpret *node)
{
    // remove all reinterpret nodes
    // and replace the left-hand side variable with the
    // original variable.

    doLog(LOG_DEBUG, "Replacing variable (%s)\n", node->m_lhs->m_identName.c_str());
    substituteOperands(node->m_lhs, node->m_op);
    replaceWithNull(node);
    return true;
}

void PassClean::substituteOperands(const SharedOpPtr op1, SharedOpPtr op2)
{
    for(auto statement : m_ssa->m_statements)
    {
        statement->replaceOperand(op1,op2);
    }
}

void PassClean::replaceWithNull(const OperationBase *node)
{
    auto iter = m_ssa->m_statements.begin();
    while(iter != m_ssa->m_statements.end())
    {
        if ((*iter) == node)
        {
            delete (*iter);
            (*iter) = new OpNull();
        }
        iter++;
    }
}




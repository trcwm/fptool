/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Clean the operand list

  1) Removes unused operands from the operand list.

  Author: Niels A. Moseley

*/

#include "logging.h"
#include "pass_removeoperands.h"

using namespace SSA;

bool PassRemoveOperands::execute(Program &ssa)
{
    doLog(LOG_INFO, "--------------------------------\n");
    doLog(LOG_INFO, "  Running Remove Operands pass\n");
    doLog(LOG_INFO, "--------------------------------\n");

    PassRemoveOperands pass(ssa);

    // reset all m_usedFlags
    for(auto operand : ssa.m_operands)
    {
        operand->m_usedFlag = false;
    }

    // set the m_usedFlag if an operand is used
    // in an SSA node.
    for(auto statement : ssa.m_statements)
    {
        if (!statement->accept(&pass))
        {
            return false;
        }
    }

    // remove all operands which have m_usedFlag == false
    auto iter = ssa.m_operands.begin();
    while(iter != ssa.m_operands.end())
    {
        if ((*iter)->m_usedFlag == false)
        {
            //doLog(LOG_INFO, "Removing %s\n", (*iter)->m_identName.c_str());
            iter = ssa.m_operands.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    return true;
}

bool PassRemoveOperands::visit(const OpAssign *node)
{
    node->m_lhs->m_usedFlag = true;
    node->m_op->m_usedFlag = true;
    return true;
}

bool PassRemoveOperands::visit(const OpNegate *node)
{
    node->m_lhs->m_usedFlag = true;
    node->m_op->m_usedFlag = true;
    return true;
}

bool PassRemoveOperands::visit(const OpMul *node)
{
    node->m_lhs->m_usedFlag = true;
    node->m_op1->m_usedFlag = true;
    node->m_op2->m_usedFlag = true;
    return true;
}

bool PassRemoveOperands::visit(const OpAdd *node)
{
    if (node->m_noExtension)
    {
        node->m_lhs->m_usedFlag = true;
        node->m_op1->m_usedFlag = true;
        node->m_op2->m_usedFlag = true;
    }
    else
    {
        // VHDL does not support automatic MSB extensions
        // for add, so we can only handle non-extended ADDs.
        //
        // MSB extension should have been handled by a
        // previous pass, but if we end up here, it
        // obviously wasn't
        return false;
    }
    return true;
}

bool PassRemoveOperands::visit(const OpSub *node)
{
    if (node->m_noExtension)
    {
        node->m_lhs->m_usedFlag = true;
        node->m_op1->m_usedFlag = true;
        node->m_op2->m_usedFlag = true;
    }
    else
    {
        // VHDL does not support automatic MSB extensions
        // for add, so we can only handle non-extended SUBs.
        //
        // MSB extension should have been handled by a
        // previous pass, but if we end up here, it
        // obviously wasn't
        return false;
    }
    return true;
}

bool PassRemoveOperands::visit(const OpExtendLSBs *node)
{
    node->m_lhs->m_usedFlag = true;
    node->m_op->m_usedFlag = true;
    return true;
}

bool PassRemoveOperands::visit(const OpExtendMSBs *node)
{
    node->m_lhs->m_usedFlag = true;
    node->m_op->m_usedFlag = true;
    return true;
}

bool PassRemoveOperands::visit(const OpRemoveMSBs *node)
{
    node->m_lhs->m_usedFlag = true;
    node->m_op->m_usedFlag = true;
    return true;
}

bool PassRemoveOperands::visit(const OpRemoveLSBs *node)
{
    node->m_lhs->m_usedFlag = true;
    node->m_op->m_usedFlag = true;
    return true;
}

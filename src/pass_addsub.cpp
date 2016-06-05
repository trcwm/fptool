/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Add/subtract LSB equalization

*/


#include "pass_addsub.h"

// make sure the fractional parts of every
// addition and subtraction is the same
void PassAddSub::execute()
{
    auto iter = m_ssaList->begin();
    while(iter != m_ssaList->end())
    {
        SSA::SSANode::operation_t operation = iter->operation;
        if ((operation == SSA::SSANode::OP_Add) || (operation == SSA::SSANode::OP_Sub))
        {
            // get operands
            SSA::operand_t op1 = getOperand(iter->var1);
            SSA::operand_t op2 = getOperand(iter->var2);

            if (op1.info.fracBits > op2.info.fracBits)
            {
                // extend LSBs of op2 by creating a new extended
                // version of op2
                uint32_t newop = createIntermediate(op2.info.intBits, op1.info.fracBits);

                // replace op2 by this new node in the current SSA node
                uint32_t old_op2 = iter->var2;
                iter->var2 = newop;

                // insert extendLSB command before this operation
                SSA::SSANode newnode;
                newnode.operation = SSA::SSANode::OP_ExtendLSBs;
                newnode.var3 = newop;
                newnode.var1 = old_op2;
                newnode.bits = op2.info.intBits;
                newnode.fbits = op1.info.fracBits;
                iter = m_ssaList->insert(iter, newnode);
            }
            else if (op2.info.fracBits > op1.info.fracBits)
            {
                // extend LSBs of op1 by creating a new extended
                // version of op1
                uint32_t newop = createIntermediate(op1.info.intBits, op2.info.fracBits);

                // replace op1 by this new node in the current SSA node
                uint32_t old_op1 = iter->var1;
                iter->var1 = newop;

                // insert extendLSB command before this operation
                SSA::SSANode newnode;
                newnode.operation = SSA::SSANode::OP_ExtendLSBs;
                newnode.var3 = newop;
                newnode.var1 = old_op1;
                newnode.bits = op1.info.intBits;
                newnode.fbits = op2.info.fracBits;
                iter = m_ssaList->insert(iter, newnode);
            }
        }
        iter++;
    }
}

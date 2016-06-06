/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Add/subtract LSB equalization

*/


#include "pass_addsub.h"

// make sure the fractional parts of every
// addition and subtraction is the same
void PassAddSub::execute(SSAObject &ssa)
{
    auto iter = ssa.begin();
    while(iter != ssa.end())
    {
        SSANode::operation_t operation = iter->operation;
        if ((operation == SSANode::OP_Add) || (operation == SSANode::OP_Sub))
        {
            // get operands
            operand_t op1 = ssa.getOperand(iter->var1);
            operand_t op2 = ssa.getOperand(iter->var2);

            if (op1.info.fracBits > op2.info.fracBits)
            {
                // extend LSBs of op2 by creating a new extended
                // version of op2
                operandIndex newop = ssa.createExtendLSBNode(iter, iter->var2, op1.info.fracBits-op2.info.fracBits);

                // replace op2 by this new node in the current SSA node
                iter->var2 = newop;
            }
            else if (op2.info.fracBits > op1.info.fracBits)
            {
                // extend LSBs of op1 by creating a new extended
                // version of op1
                operandIndex newop = ssa.createExtendLSBNode(iter, iter->var1, op2.info.fracBits-op1.info.fracBits);

                // replace op1 by this new node in the current SSA node
                iter->var1 = newop;
            }
        }
        iter++;
    }
}

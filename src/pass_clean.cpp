/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Clean the SSA list

  1) remove re-interpret nodes
  2) remove superluous assignment nodes

*/

#include "pass_clean.h"

// make sure the fractional parts of every
// addition and subtraction is the same
void PassClean::execute(SSAObject &ssa)
{
    // remove re-interpreted nodes
    auto iter = ssa.begin();
    while(iter != ssa.end())
    {
        SSANode::operation_t operation = iter->operation;

        if (operation == SSANode::OP_Reinterpret)
        {
            // replace reinterpreted with original variable
            substitute(ssa, iter->var3, iter->var1);

            // mark lhs variable as removed
            ssa.markRemoved(iter->var3);

            // remove the reinterpret node
            iter = ssa.removeNode(iter);
        }
        else
        {
            iter++;
        }
    }

    // remove superfluous assignments
    iter = ssa.begin();
    while(iter != ssa.end())
    {
        SSANode::operation_t operation = iter->operation;

        if (operation == SSANode::OP_Assign)
        {
            operand_t lhs = ssa.getOperand(iter->var3);

            // preserve output assignments
            if (lhs.type != operand_t::TypeOutput)
            {
                // replace the assigned var with original var
                substitute(ssa, iter->var3, iter->var1);

                // mark lhs variable as removed
                ssa.markRemoved(iter->var3);

                // remove the assignment node
                iter = ssa.removeNode(iter);
            }
            else
            {
                iter++;
            }
        }
        else
        {
            iter++;
        }
    }
}

void PassClean::substitute(SSAObject &ssa, operandIndex idx1, operandIndex idx2)
{
    auto iter = ssa.begin();
    while(iter != ssa.end())
    {
        if (iter->var1 == idx1)
            iter->var1 = idx2;

        if (iter->var2 == idx1)
            iter->var2 = idx2;

        iter++;
    }
}

/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Clean the SSA list

  1) remove re-interpret nodes
  2) remove superluous assignment nodes

*/

#include "logging.h"
#include "pass_clean.h"

// make sure the fractional parts of every
// addition and subtraction is the same
void PassClean::execute(SSAObject &ssa)
{
    doLog(LOG_INFO, "Running Clean pass\n");

    // remove re-interpreted nodes
    auto iter = ssa.begin();
    while(iter != ssa.end())
    {
        SSANode::operation_t operation = iter->operation;

        if (operation == SSANode::OP_Reinterpret)
        {
            doLog(LOG_DEBUG, "Replacing variable %d (%s)\n", iter->op3Idx, ssa.getOperand(iter->op3Idx).info.txt.c_str());

            // replace reinterpreted with original variable
            substitute(ssa, iter->op3Idx, iter->op1Idx);

            // mark lhs variable as removed
            ssa.markRemoved(iter->op3Idx);

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
            operand_t lhs = ssa.getOperand(iter->op3Idx);

            // preserve output assignments
            if (lhs.type != operand_t::TypeOutput)
            {
                doLog(LOG_DEBUG, "Removing variable %d (%s)\n", iter->op3Idx, ssa.getOperand(iter->op3Idx).info.txt.c_str());

                // replace the assigned var with original var
                substitute(ssa, iter->op3Idx, iter->op1Idx);

                // mark lhs variable as removed
                ssa.markRemoved(iter->op3Idx);

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
        if (iter->op1Idx == idx1)
            iter->op1Idx = idx2;

        if (iter->op2Idx == idx1)
            iter->op2Idx = idx2;

        iter++;
    }
}

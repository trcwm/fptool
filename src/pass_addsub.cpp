/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Add/subtract LSB equalization

*/

#include "logging.h"
#include "pass_addsub.h"

// make sure the fractional parts of every
// addition and subtraction is the same
// and sign-extend the arguments to overflow never occurs
//
void PassAddSub::execute(SSAObject &ssa)
{
    doLog(LOG_INFO, "-----------------------\n");
    doLog(LOG_INFO, "  Running AddSub pass\n");
    doLog(LOG_INFO, "-----------------------\n");

    auto iter = ssa.begin();
    while(iter != ssa.end())
    {
        SSANode::operation_t operation = iter->operation;
        if ((operation == SSANode::OP_Add) || (operation == SSANode::OP_Sub))
        {
            // get operands
            operand_t op1 = ssa.getOperand(iter->op1Idx);
            operand_t op2 = ssa.getOperand(iter->op2Idx);

            doLog(LOG_DEBUG, "Processing (%s) and (%s) for addition\n", op1.info.txt.c_str(), op2.info.txt.c_str());

            // **********************************************************************
            //   Equalise the LSBs/fractional part
            // **********************************************************************
            if (op1.info.fracBits > op2.info.fracBits)
            {
                // extend LSBs of op2 by creating a new extended
                // version of op2
                operandIndex newop = ssa.createExtendLSBNode(iter, iter->op2Idx, op1.info.fracBits-op2.info.fracBits);

                // replace op2 by this new node in the current SSA node
                iter->op2Idx = newop;
            }
            else if (op2.info.fracBits > op1.info.fracBits)
            {
                // extend LSBs of op1 by creating a new extended
                // version of op1
                operandIndex newop = ssa.createExtendLSBNode(iter, iter->op1Idx, op2.info.fracBits-op1.info.fracBits);

                // replace op1 by this new node in the current SSA node
                iter->op1Idx = newop;
            }

            // **********************************************************************
            //   Extend both arguments by 1 MSB to avoid overflow
            // **********************************************************************

            // extend largest argument by one MSB
            // of, if there is no largets argument
            // pick op1.
            if (op1.info.intBits >= op2.info.intBits)
            {
                // extend MSBs of op1 by creating a new extended
                // version of op1
                operandIndex newop = ssa.createExtendMSBNode(iter, iter->op1Idx, 1);

                // replace op1 by this new node in the current SSA node
                iter->op1Idx = newop;
            }
            else if (op2.info.intBits > op1.info.intBits)
            {
                // extend MSBs of op2 by creating a new extended
                // version of op2
                operandIndex newop = ssa.createExtendMSBNode(iter, iter->op2Idx, 1);

                // replace op1 by this new node in the current SSA node
                iter->op2Idx = newop;
            }
        }
        iter++;
    }
}

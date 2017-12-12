/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Handle truncate function

*/

#include "logging.h"
#include "pass_truncate.h"

// turn truncate function node into
// OP_RemoveLSBs/OP_ExtendLSBs and
// OP_RemoveMSBs/OP_ExtendMSBs nodes.

void PassTruncate::execute(SSAObject &ssa)
{
    doLog(LOG_INFO, "-------------------------\n");
    doLog(LOG_INFO, "  Running Truncate pass\n");
    doLog(LOG_INFO, "-------------------------\n");

    auto iter = ssa.begin();
    while(iter != ssa.end())
    {
        SSANode::operation_t operation = iter->operation;
        if (operation == SSANode::OP_Truncate)
        {
            // get operand
            operand_t op1 = ssa.getOperand(iter->op1Idx);

            doLog(LOG_DEBUG, "Processing truncation of (%s)\n", op1.info.txt.c_str());

            // **********************************************************************
            //   Handle LSBs
            // **********************************************************************
            if (op1.info.fracBits > iter->fbits)
            {
                // truncate the LSBs
                operandIndex newop = ssa.createRemoveLSBNode(iter, iter->op1Idx, op1.info.fracBits - iter->fbits);

                // replace op2 by this new node in the current SSA node
                iter->op1Idx = newop;
            }
            else if (op1.info.fracBits < iter->fbits)
            {
                // extend LSBs of op1
                // version of op1
                operandIndex newop = ssa.createExtendLSBNode(iter, iter->op1Idx, iter->fbits-op1.info.fracBits);

                // replace op1 by this new node in the current SSA node
                iter->op1Idx = newop;
            }
            else
            {
                // no LSBs need to be harmed.
            }

            // **********************************************************************
            //   Handle MSBs
            // **********************************************************************

            if (op1.info.intBits > iter->bits)
            {
                // truncate the MSBs
                operandIndex newop = ssa.createRemoveMSBNode(iter, iter->op1Idx, op1.info.intBits - iter->bits);

                // replace op1 by this new node in the current SSA node
                iter->op1Idx = newop;
            }
            else if (op1.info.intBits < iter->bits)
            {
                // extend the MSBs
                operandIndex newop = ssa.createExtendMSBNode(iter, iter->op1Idx, iter->bits-op1.info.intBits);

                // replace op1 by this new node in the current SSA node
                iter->op1Idx = newop;
            }
            else
            {
                // no MSBs need to be harmed
            }

            // finally, replace the truncate node with a simple assignment
            // for this, we abuse the reinterpret node
            iter->operation = SSANode::OP_Reinterpret;
        }
        iter++;
    }
}

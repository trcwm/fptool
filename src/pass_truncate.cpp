/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Handle truncate function

*/

#include "logging.h"
#include "pass_truncate.h"

using namespace SSA;

bool PassTruncate::execute(Program &ssa)
{
    doLog(LOG_INFO, "-------------------------\n");
    doLog(LOG_INFO, "  Running Truncate pass\n");
    doLog(LOG_INFO, "-------------------------\n");

    PassTruncate pass(ssa);

    for(auto operation : ssa.m_statements)
    {
        if (!operation->accept(&pass))
        {
            return false;
        }
    }

    ssa.applyPatches(); // integrate the generate OpPatchBlock instructions.
    ssa.updateOutputPrecisions();
    return true;
}

void PassTruncate::patchNode(const SSA::OperationBase *node, SSA::OpPatchBlock *patch)
{
    auto iter = std::find(m_ssa->m_statements.begin(), m_ssa->m_statements.end(), node);
    if (iter != m_ssa->m_statements.end())
    {
        *iter = patch;

        //FIXME: we should get rid of the node
        //       but we can't do that safely here as there
        //       could be iterators accessing it.
        //       perhaps we need a shared pointer?
        //       for now, we'll just let it leak.
        //delete node;
    }
}

bool PassTruncate::visit(const OpTruncate *node)
{
    doLog(LOG_DEBUG, "Processing truncation of (%s)\n", node->m_op->m_identName.c_str());

    OpPatchBlock *patch = new OpPatchBlock(node);

    SharedOpPtr inOp = node->m_op;

    // **********************************************************************
    //   Handle LSBs
    // **********************************************************************
    if (node->m_op->m_fracBits > node->m_fracBits)
    {
        // truncate the LSBs
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        OpRemoveLSBs* instr = new OpRemoveLSBs(inOp, tmp, node->m_op->m_fracBits - node->m_fracBits);
        patch->addStatement(instr);

        // replace the input operand with the new temporary output
        inOp = tmp;
    }
    else if (node->m_op->m_fracBits < node->m_fracBits)
    {
        // extend the LSBs
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        OpExtendLSBs* instr = new OpExtendLSBs(inOp, tmp, node->m_fracBits - node->m_op->m_fracBits);
        patch->addStatement(instr);

        // replace the input operand with the new temporary output
        inOp = tmp;
    }
    else
    {
        // no LSBs need to be harmed.
    }

    // **********************************************************************
    //   Handle MSBs
    // **********************************************************************

    if (node->m_op->m_intBits > node->m_intBits)
    {
        // truncate the MSBs
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        OpRemoveMSBs* instr = new OpRemoveMSBs(inOp, tmp, node->m_op->m_intBits - node->m_intBits);
        patch->addStatement(instr);

        // replace the input operand with the new temporary output
        inOp = tmp;
    }
    else if (node->m_op->m_intBits < node->m_intBits)
    {
        // extend the MSBs
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        OpExtendMSBs* instr = new OpExtendMSBs(inOp, tmp, node->m_intBits - node->m_op->m_intBits);
        patch->addStatement(instr);

        // replace the input operand with the new temporary output
        inOp = tmp;
    }
    else
    {
        // no MSBs need to be harmed
    }


    OpAssign* instr = new OpAssign(inOp, node->m_lhs);
    patch->addStatement(instr);
    patchNode(node, patch);

    return true;
}

#if 0
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
#endif


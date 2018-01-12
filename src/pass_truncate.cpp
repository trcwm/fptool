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
        m_ssa->addOperand(tmp);
        patch->addStatement(instr);

        // replace the input operand with the new temporary output
        inOp = tmp;
    }
    else if (node->m_op->m_fracBits < node->m_fracBits)
    {
        // extend the LSBs
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        OpExtendLSBs* instr = new OpExtendLSBs(inOp, tmp, node->m_fracBits - node->m_op->m_fracBits);
        m_ssa->addOperand(tmp);
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
        m_ssa->addOperand(tmp);
        patch->addStatement(instr);

        // replace the input operand with the new temporary output
        inOp = tmp;
    }
    else if (node->m_op->m_intBits < node->m_intBits)
    {
        // extend the MSBs
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        OpExtendMSBs* instr = new OpExtendMSBs(inOp, tmp, node->m_intBits - node->m_op->m_intBits);
        m_ssa->addOperand(tmp);
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

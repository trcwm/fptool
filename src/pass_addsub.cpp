/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Add/subtract LSB equalization

*/

#include "logging.h"
#include "pass_addsub.h"

using namespace SSA;

bool PassAddSub::execute(Program &ssa)
{
    doLog(LOG_INFO, "-----------------------\n");
    doLog(LOG_INFO, "  Running AddSub pass\n");
    doLog(LOG_INFO, "-----------------------\n");

    PassAddSub pass(ssa);

    for(auto operation : ssa.m_statements)
    {
        if (!operation->accept(&pass))
        {
            return false;
        }
    }

    ssa.applyPatches(); // integrate the generate OpPatchBlock instructions.
    return true;
}

bool PassAddSub::visit(const OpAdd *node)
{
    doLog(LOG_DEBUG, "Processing (%s) and (%s) for addition\n",
          node->m_op1->m_identName.c_str(),
          node->m_op2->m_identName.c_str());

    OpPatchBlock *patch = new OpPatchBlock(node);

    SharedOpPtr op1 = node->m_op1;
    SharedOpPtr op2 = node->m_op2;
    // **********************************************************************
    //   Equalise the LSBs/fractional part
    // **********************************************************************
    if (op1->m_fracBits > op2->m_fracBits)
    {
        // extend LSBs of op2 by creating a new extended
        // version of op2
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        SSA::OpExtendLSBs *instr = new SSA::OpExtendLSBs(op2, tmp, op1->m_fracBits - op2->m_fracBits);
        patch->m_statements.push_back(instr);
        m_ssa->addOperand(tmp);

        // replace op2 by this new node in the current SSA node
        //node->m_op2.reset();
        op2 = tmp;
    }
    else if (op2->m_fracBits > op1->m_fracBits)
    {
        // extend LSBs of op1 by creating a new extended
        // version of op1
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        SSA::OpExtendLSBs *instr = new SSA::OpExtendLSBs(op1, tmp, op2->m_fracBits - op1->m_fracBits);
        patch->m_statements.push_back(instr);
        m_ssa->addOperand(tmp);
        op1 = tmp;
    }

    // **********************************************************************
    //   Extend largest argument by 1 MSB to avoid overflow
    // **********************************************************************

    // if there is no largets argument
    // pick op1.
    if (op1->m_intBits >= op2->m_intBits)
    {
        // extend MSBs of op1 by creating a new extended
        // version of op1
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        SSA::OpExtendMSBs *instr = new SSA::OpExtendMSBs(op1, tmp, 1);
        patch->m_statements.push_back(instr);
        m_ssa->addOperand(tmp);
        op1 = tmp;
    }
    else
    {
        // extend MSBs of op1 by creating a new extended
        // version of op2
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        SSA::OpExtendMSBs *instr = new SSA::OpExtendMSBs(op2, tmp, 1);
        patch->m_statements.push_back(instr);
        m_ssa->addOperand(tmp);
        op2 = tmp;
    }

    // patch the instruction if there are actually instructions
    // in the patch!
    //
    // the patch will replace the add instruction itself
    // so we need to add that instruction in the patch block

    if (patch->m_statements.size() > 0)
    {
        // replace the original add instruction

        //FIXME: the MSB extension causes the add
        //       operation to overestimate the
        //       resulting Q(n,m) specification.
        //
        //       we need to fix this by not adjusting
        //       the Q(n,m) of the output operand
        //       in the constructor of the operations
        //       but rather determine them by calculating
        //       the min/max values of the result.
        //
        //       for now, we'll just copy the old settings
        //       of the int/frac bits from the original
        //       result as a quick fix.

        //int32_t outIntBits = node->m_lhs->m_intBits;
        //int32_t outFracBits = node->m_lhs->m_fracBits;
        SSA::OpAdd *add = new SSA::OpAdd(op1,op2, node->m_lhs);
        //node->m_lhs->m_intBits = outIntBits;
        //node->m_lhs->m_fracBits = outFracBits;

        patch->m_statements.push_back(add);
        patchNode(node, patch);
    }
    return true;
}

bool PassAddSub::visit(const OpSub *node)
{
    doLog(LOG_DEBUG, "Processing (%s) and (%s) for subtraction\n",
          node->m_op1->m_identName.c_str(),
          node->m_op2->m_identName.c_str());

    OpPatchBlock *patch = new OpPatchBlock(node);

    SharedOpPtr op1 = node->m_op1;
    SharedOpPtr op2 = node->m_op2;
    // **********************************************************************
    //   Equalise the LSBs/fractional part
    // **********************************************************************
    if (op1->m_fracBits > op2->m_fracBits)
    {
        // extend LSBs of op2 by creating a new extended
        // version of op2
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        SSA::OpExtendLSBs *instr = new SSA::OpExtendLSBs(op2, tmp, op1->m_fracBits - op2->m_fracBits);
        patch->m_statements.push_back(instr);
        m_ssa->addOperand(tmp);

        // replace op2 by this new node in the current SSA node
        //node->m_op2.reset();
        op2 = tmp;
    }
    else if (op2->m_fracBits > op1->m_fracBits)
    {
        // extend LSBs of op1 by creating a new extended
        // version of op1
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        SSA::OpExtendLSBs *instr = new SSA::OpExtendLSBs(op1, tmp, op2->m_fracBits - op1->m_fracBits);
        patch->m_statements.push_back(instr);
        m_ssa->addOperand(tmp);
        op1 = tmp;
    }

    // **********************************************************************
    //   Extend largest argument by 1 MSB to avoid overflow
    // **********************************************************************

    // if there is no largets argument
    // pick op1.
    if (op1->m_intBits >= op2->m_intBits)
    {
        // extend MSBs of op1 by creating a new extended
        // version of op1
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        SSA::OpExtendMSBs *instr = new SSA::OpExtendMSBs(op1, tmp, 1);
        patch->m_statements.push_back(instr);
        m_ssa->addOperand(tmp);
        op1 = tmp;
    }
    else
    {
        // extend MSBs of op1 by creating a new extended
        // version of op2
        SharedOpPtr tmp = IntermediateOperand::createNewIntermediate();
        SSA::OpExtendMSBs *instr = new SSA::OpExtendMSBs(op2, tmp, 1);
        patch->m_statements.push_back(instr);
        m_ssa->addOperand(tmp);
        op2 = tmp;
    }

    // patch the instruction if there are actually instructions
    // in the patch!
    //
    // the patch will replace the add instruction itself
    // so we need to add that instruction in the patch block

    if (patch->m_statements.size() > 0)
    {
        // replace the original add instruction

        //FIXME: the MSB extension causes the add
        //       operation to overestimate the
        //       resulting Q(n,m) specification.
        //
        //       we need to fix this by not adjusting
        //       the Q(n,m) of the output operand
        //       in the constructor of the operations
        //       but rather determine them by calculating
        //       the min/max values of the result.
        //
        //       for now, we'll just copy the old settings
        //       of the int/frac bits from the original
        //       result as a quick fix.
        //int32_t outIntBits = node->m_lhs->m_intBits;
        //int32_t outFracBits = node->m_lhs->m_fracBits;
        SSA::OpSub *sub = new SSA::OpSub(op1,op2, node->m_lhs);
        //node->m_lhs->m_intBits = outIntBits;
        //node->m_lhs->m_fracBits = outFracBits;

        patch->m_statements.push_back(sub);
        patchNode(node, patch);
    }
    return true;
}

void PassAddSub::patchNode(const SSA::OperationBase *node, SSA::OpPatchBlock *patch)
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

#if 0
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
#endif

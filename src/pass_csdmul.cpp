/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

*/

#include <memory>
#include <iostream>
#include <sstream>
#include "logging.h"
#include "csd.h"
#include "ssaprint.h"
#include "pass_csdmul.h"

using namespace SSA;


bool PassCSDMul::execute(Program &ssa)
{
    doLog(LOG_INFO, "-----------------------\n");
    doLog(LOG_INFO, "  Running CSDMul pass\n");
    doLog(LOG_INFO, "-----------------------\n");

    PassCSDMul pass(ssa);

    // look for CSD * variable, variable * CSD
    // or CSD * CSD
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

void PassCSDMul::patchNode(const SSA::OperationBase *node, SSA::OpPatchBlock *patch)
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

bool PassCSDMul::visit(const OpCSDMul *node)
{
    doLog(LOG_INFO, "Expanding CSD %s\n", node->m_csdName.c_str());

    OpPatchBlock *patch = new OpPatchBlock(node);
    expandCSD(node->m_csd, node->m_op, node->m_lhs, patch, m_ssa->m_operands);
    patchNode(node, patch); // replace the MUL node.
    return true;
}

bool PassCSDMul::visit(const OpMul *node)
{
    // check that there are no MUL nodes that
    // have a CSD operands
    if (node->m_op1->isCSD() || node->m_op2->isCSD())
    {
        doLog(LOG_ERROR, "One or more OpMul arguments are of type CSD (%s) (%s)\n",
              node->m_op1->m_identName.c_str(),
              node->m_op2->m_identName.c_str());

        // both operands are CSD!
        // TODO: think of a better strategy
        // FIXME: produce a warning
        // for now, we bail with an error
        //throw std::runtime_error("PassCSDMul: both operands are CSD -- unsupported");
        return false;
    }

    // no CSD operands found -> nothing to do.
    return true;
}

void PassCSDMul::expandCSD(const csd_t &csd,
                           const SharedOpPtr &input,
                           const SharedOpPtr &output,
                           SSA::OpPatchBlock *patch,
                           std::list<SharedOpPtr> &operands)
{
    // the procedure is as follows:
    //
    // y = csd*input
    //
    // 1) create the first shift variable
    //    this variable is nothing more
    //    than a re-interpretation of the
    //    input variable: only the Q(n,m)
    //    changes to Q(n+shift,m+shift)
    //
    //    note that the shift is negative
    //    for multiplication with a fractional
    //    digit of 'csd'.
    //
    // 2) create the second shift variable
    //    by the same procedure. Now, we
    //    need to extend LSBs to make
    //    sure the binary point of both
    //    terms are aligned so we can
    //    add or subtract them
    //
    // 3) store the result in a new
    //    temporary variable and insert
    //    the addition/subtraction in the
    //    SSA list.
    //
    // 4) goto 2 while there are still
    //    digits that need processing.

    // If the CSD is negative
    // make it positive and insert a negation node
    // at the output, which then can be absorbed
    // into an addition or subtraction in later
    // optimization passes.

    // get the digit with the smallest
    // power.
    auto digitIter = csd.digits.rbegin();   // start at the last digit (smallest power)
    if (digitIter == csd.digits.rend())
    {
        // CSD has no digits!
        // FIXME: change handling with error() function
        //        and log support
        throw std::runtime_error("CSD has no digits!");

        //return;
    }

    int32_t shift = digitIter->power;

    // create first shifted version of
    // input and insert it into the operand
    // list.
    SharedOpPtr result = IntermediateOperand::createNewIntermediate();
    SSA::OpReinterpret *reinterpret = new SSA::OpReinterpret(input,
                                                             result,
                                                             input->m_intBits+shift,
                                                             input->m_fracBits-shift);
    patch->m_statements.push_back(reinterpret);
    operands.push_back(result);

    // if the first digit is negative, we need to
    // insert a negation operation becuase the first
    // digit is always the left operand of the
    // + or - operation: it will always be
    // taken as a positive value / input.
    if (digitIter->sign < 0)
    {
        SharedOpPtr insertResult = IntermediateOperand::createNewIntermediate();
        SSA::OpNegate *negate = new SSA::OpNegate(result, insertResult);
        patch->m_statements.push_back(negate);
        operands.push_back(insertResult);
        result = insertResult;
    }

    digitIter++;

    // while there are digits in CSD
    // keep on adding new terms
    SharedOpPtr t1 = result;
    while(digitIter != csd.digits.rend())
    {
        // create new term
        shift = digitIter->power;

        SharedOpPtr t2 = IntermediateOperand::createNewIntermediate();
        reinterpret = new SSA::OpReinterpret(input,
                                             t2,
                                             input->m_intBits+shift,
                                             input->m_fracBits-shift);
        operands.push_back(t2);
        patch->m_statements.push_back(reinterpret);

        // add the terms
        result = IntermediateOperand::createNewIntermediate();
        operands.push_back(result);
        if (digitIter->sign > 0)
        {
            // the sum of t1 and t2 will always be larger in
            // magnitude so we _do_ need an additional sign extension bit.
            SSA::OpAdd *adder = new SSA::OpAdd(t1,t2,result);
            patch->m_statements.push_back(adder);
        }
        else
        {
            // the weight of t2 is always smaller than t1
            // so the result of t1-t2 will always be smaller in
            // magnitude than t1. As a result, we do not need an addition
            // sign extension bit.
            SSA::OpSub *subber = new SSA::OpSub(t1,t2,result, true);
            patch->m_statements.push_back(subber);
        }
        t1 = result;
        digitIter++;
    }

#if 0
    // sanity check: the output size and temporary
    // variable t1 must match!
    operand_t outputOp = ssa.getOperand(y_idx);
    operand_t tempOp   = ssa.getOperand(t1);
    if ((outputOp.info.fracBits != tempOp.info.fracBits) || (outputOp.info.intBits != tempOp.info.intBits))
    {
        doLog(LOG_WARN, "CSD output operand size mismatch: \n");
        doLog(LOG_WARN, "  output: %s Q(%d,%d)\n", outputOp.info.txt.c_str(), outputOp.info.intBits, outputOp.info.fracBits);
        doLog(LOG_WARN, "  temp  : %s Q(%d,%d)\n", tempOp.info.txt.c_str(), tempOp.info.intBits, tempOp.info.fracBits);
        doLog(LOG_WARN, "Creating truncation statement to fix this ..\n");
        // Note: this is most likely ok, because the CSD constant will not cause overflow
        //       as a work-around we insert a truncate node
        t1 = ssa.createTruncateNode(ssa_iter, t1, outputOp.info.intBits, outputOp.info.fracBits);
    }
#endif

    // make the final assignment
    SSA::OpAssign *assign = new SSA::OpAssign(result, output);
    patch->m_statements.push_back(assign);
}

/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

*/

#include "logging.h"
#include "csd.h"
#include "pass_csdmul.h"


void PassCSDMul::execute(SSAObject &ssa)
{
    doLog(LOG_INFO, "-----------------------\n");
    doLog(LOG_INFO, "  Running CSDMul pass\n");
    doLog(LOG_INFO, "-----------------------\n");

    auto iter = ssa.begin();

    // look for CSD * variable, variable * CSD
    // or CSD * CSD
    while(iter != ssa.end())
    {
        SSANode::operation_t operation = iter->operation;
        if (operation == SSANode::OP_Mul)
        {
            // check both operands for CSD
            operand_t op1 = ssa.getOperand(iter->op1Idx);
            operand_t op2 = ssa.getOperand(iter->op2Idx);

            if ((op1.type == operand_t::TypeCSD) && (op2.type == operand_t::TypeCSD))
            {
                doLog(LOG_WARN, "Both arguments are of type CSD (%s) (%s)\n", op1.info.txt.c_str(), op2.info.txt.c_str());
                // both operands are CSD!
                // TODO: think of a better strategy
                // FIXME: produce a warning
                // for now, we bail with an error
                throw std::runtime_error("PassCSDMul: both operands are CSD -- unsupported");
            }

            doLog(LOG_DEBUG, "Processing (%s) and (%s) for CSD expansion\n", op1.info.txt.c_str(), op2.info.txt.c_str());

            // check if operand 2 is the CSD
            // if so, swap operands as this
            // simplifies the code ahead
            operandIndex varIdx = iter->op2Idx;
            if (op2.type == operand_t::TypeCSD)
            {
                varIdx = iter->op1Idx;
                std::swap(op1, op2);
            }


            csd_t my_csd;
            if (op1.type == operand_t::TypeCSD)
            {
                if (!convertToCSD(op1.info.csdFloat, op1.info.csdBits, my_csd))
                {
                    doLog(LOG_ERROR, "Cannot convert number (%f) to CSD\n", op1.info.csdFloat);
                    throw std::runtime_error("");
                }
                doLog(LOG_DEBUG, "CSD: converted (%f) to (%f)\n", op1.info.csdFloat, my_csd.value);
                iter = shiftAndAdd(ssa, iter, my_csd, varIdx, iter->op3Idx);

                // the new iter will now point to the next statement:
                // we must use continue here to avoid calling the iter++ later on
                // as this will skip the statement directly following this one.
                continue;
            }

            // TODO: check if the final result matches the width of the
            // destination. If no match, there was a problem
            // in determining the wordlength earlier in the
            // process.
        }
        iter++;
    }
}

ssa_iterator PassCSDMul::shiftAndAdd(SSAObject &ssa,
                                     ssa_iterator ssa_iter,
                                     const csd_t &csd,
                                     uint32_t x_idx,
                                     uint32_t y_idx)
{
    // the procedure is as follows:
    //
    // y = c*x
    //
    // 1) create the first shift variable
    //    this variable is nothing more
    //    than a re-interpretation of the
    //    input variable 'x': only the Q(n,m)
    //    changes to Q(n+shift,m+shift)
    //
    //    note that the shift is negative
    //    for multiplication with a fractional
    //    digit of 'c'.
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

    if (csd.digits.size() < 1)
    {
        throw std::runtime_error("PassCSDMul: can't expand a CSD without digits!");
    }

    // remove the current assigment as we're replacing it.
    // the iterator now points to the next (unrelated)
    // operation. We must insert new nodes before this
    // operation, which is exactly what the 'create'
    // functions do. :)
    ssa_iter = ssa.removeNode(ssa_iter);

    // note: the assumption is
    // that var1 the CSD 'c' and
    // var2 is 'x'.

    // get the digit with the smallest
    // power.
    int32_t idx = static_cast<int32_t>(csd.digits.size()-1);
    int32_t shift = csd.digits[idx].power;

    // create first shifted version of
    // x and insert it into the operand
    // list. The assign the value to it.

    // FIXME: what if the first digit is negative?
    operand_t x = ssa.getOperand(x_idx);
    operandIndex t1 = ssa.createReinterpretNode(ssa_iter, x_idx, x.info.intBits+shift, x.info.fracBits-shift);

    // check if the first digit is negative
    // if so, we need to negate it first.
    //
    // FIXME: we need to add protection against overflows here
    // as negating the most negative number cannot be
    // represented as a positive number in 2's complement
    // arithmetic!
    if (csd.digits[idx].sign < 0)
    {
        t1 = ssa.createNegateNode(ssa_iter, t1);
    }

    // while there are digits in CSD
    // keep on adding new terms
    idx--;
    while(idx >= 0)
    {
        // create new term
        shift = csd.digits[idx].power;
        uint32_t t2 = ssa.createReinterpretNode(ssa_iter, x_idx, x.info.intBits+shift, x.info.fracBits-shift);

        // add the terms
        if (csd.digits[idx].sign > 0)
            t1 = ssa.createAddNode(ssa_iter, t1, t2);
        else
            t1 = ssa.createSubNode(ssa_iter, t1, t2);

        idx--;
    }

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

    // make the final assignment
    ssa.createAssignNode(ssa_iter, y_idx, t1);


    return ssa_iter;
}



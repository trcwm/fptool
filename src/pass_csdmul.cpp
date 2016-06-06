/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

*/

#include "csd.h"
#include "pass_csdmul.h"


void PassCSDMul::execute(SSAObject &ssa)
{
    auto iter = ssa.begin();

    // look for CSD * variable, variable * CSD
    // or CSD * CSD
    while(iter != ssa.end())
    {
        SSANode::operation_t operation = iter->operation;
        if (operation == SSANode::OP_Mul)
        {
            // check both operands for CSD
            operand_t op1 = ssa.getOperand(iter->var1);
            operand_t op2 = ssa.getOperand(iter->var2);

            if ((op1.type == operand_t::TypeCSD) && (op2.type == operand_t::TypeCSD))
            {
                // both operands are CSD!
                // TODO: think of a better strategy
                // FIXME: produce a warning
                // for now, we bail with an error
                throw std::runtime_error("PassCSDMul: both operands are CSD -- unsupported");
            }

            // convert the number to a CSD represenation
            // FIXME: do this somewhere better up the
            // hierarchy!

            // check if operand 2 is the CSD
            // if so, swap operands as this
            // simplifies the code ahead
            if (op2.type == operand_t::TypeCSD)
            {
                std::swap(iter->var1, iter->var2);
                std::swap(op1, op2);
            }


            csd_t my_csd;
            if (op1.type == operand_t::TypeCSD)
            {
                if (!convertToCSD(op1.info.csdFloat, op1.info.csdBits, my_csd))
                {
                    throw std::runtime_error("PassCSDMul: cannot convert number to CSD");
                }
                operand_t result;
                iter = shiftAndAdd(ssa, iter, my_csd, iter->var2, iter->var3);
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

    // remove the current assigment as we're replacing it
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

    // make the final assignment
    ssa.createAssignNode(ssa_iter, y_idx, t1);

    return ssa_iter;
}



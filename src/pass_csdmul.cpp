/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

*/

#include "csd.h"
#include "pass_csdmul.h"

#if 0

void PassCSDMul::execute()
{
    auto iter = m_ssaList->begin();

    // look for CSD * variable, variable * CSD
    // or CSD * CSD
    while(iter != m_ssaList->end())
    {
        SSANode::operation_t operation = iter->operation;
        if (operation == SSANode::OP_Mul)
        {
            // check both operands for CSD
            operand_t op1 = getOperand(iter->var1);
            operand_t op2 = getOperand(iter->var2);

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
                shiftAndAdd(iter, my_csd, iter->var2, result);
            }

            // TODO: check if the final result matches the width of the
            // destination. If no match, there was a problem
            // in determining the wordlength earlier in the
            // process.
        }
        iter++;
    }
}

ssa_iterator PassCSDMul::shiftAndAdd(ssa_iterator ssa_iter, const csd_t &csd, uint32_t &x_idx, const operand_t &result)
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
    operand_t x = getOperand(x_idx);
    uint32_t t1 = createIntermediate(x.info.intBits+shift, x.info.fracBits+shift);
    insertReinterpretNode(ssa_iter, x_idx, t1);

    // while there are digits in CSD
    // keep on adding new terms
    idx--;
    while(idx >= 0)
    {
        // create new term
        shift = csd.digits[idx].power;
        uint32_t t2 = createIntermediate(x.info.intBits+shift, x.info.fracBits+shift);

        // extend the new term to have the same
        // number of fractional bits so we can
        // add/subtract them
        operand_t t1_op = getOperand(t1);
        operand_t t2_op = getOperand(t2);
        uint32_t t3 = createIntermediate(t2_op.info.intBits, t1_op.info.fracBits);
        operand_t t3_op = getOperand(t3);

        ssa_iter = insertExtendNode(ssa_iter, t2, t3, t2_op.info.fracBits-t3_op.info.fracBits);
        uint32_t t4 = createIntermediate(t2_op.info.intBits+1, t1_op.info.fracBits);
        operand_t t4_op = getOperand(t3);
        if (csd.digits[idx].sign > 0)
        {
            ssa_iter = insertAddNode(ssa_iter, )
        }
        else
        {

        }

    }
}

ssa_iterator PassCSDMul::insertReinterpretNode(ssa_iterator ssa_iter, uint32_t src, uint32_t dst)
{
    SSANode n;
    n.operation = SSANode::OP_Reinterpret;
    n.var3 = dst;
    n.var1 = src;
    return m_ssaList->insert(ssa_iter, n);
}

ssa_iterator PassCSDMul::insertAddNode(ssa_iterator ssa_iter, uint32_t src1, uint32_t src2, uint32_t dst)
{
    SSANode n;
    n.operation = SSANode::OP_Add;
    n.var3 = dst;
    n.var1 = src1;
    n.var2 = src2;
    return m_ssaList->insert(ssa_iter, n);
}

ssa_iterator PassCSDMul::insertSubNode(ssa_iterator ssa_iter, uint32_t src1, uint32_t src2, uint32_t dst)
{
    SSANode n;
    // dst = src2 - src1
    n.operation = SSANode::OP_Sub;
    n.var3 = dst;
    n.var1 = src2;
    n.var2 = src1;
    return m_ssaList->insert(ssa_iter, n);
}

ssa_iterator PassCSDMul::insertExtendNode(ssa_iterator ssa_iter, uint32_t src, uint32_t dst, int32_t bits)
{
    SSANode n;
    // dst = src2 - src1
    n.operation = SSANode::OP_ExtendLSBs;
    n.bits = bits;
    n.var3 = dst;
    n.var1 = src;
    return m_ssaList->insert(ssa_iter, n);
}

#endif

/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

  Each multiplication with a CSD constant is
  expanded by introducting an addition or
  subtraction of shifted inputs for each digit.

*/

#ifndef csdmul_h
#define csdmul_h

#include <vector>
#include "ssapass.h"


#if 0
class PassCSDMul : public SSAPass
{
protected:
    virtual void execute();

    /** Add a shift-and-add operation to the SSA list.
        The assumption is that var1 holds the CSD and
        var2 hold the variable. Var3 is the final
        output variable.
    */
    ssa_iterator shiftAndAdd(SSA::ssa_iterator ssa_iter, const csd_t &csd, uint32_t &x_idx, const operand_t &result);

    /** insert a reinterpret SSA node stating dst = src */
    ssa_iterator insertReinterpretNode(SSA::ssa_iterator ssa_iter, uint32_t src, uint32_t dst);

    /** insert an addition node: dst = src1 + src2 */
    ssa_iterator insertAddNode(SSA::ssa_iterator ssa_iter, uint32_t src1, uint32_t src2, uint32_t dst);

    /** insert a subtract node: dst = src1 - src2 */
    ssa_iterator insertSubNode(SSA::ssa_iterator ssa_iter, uint32_t src1, uint32_t src2, uint32_t dst);

    /** insert an Extend LSB node dst = extendLSB(src, bits) */
    ssa_iterator insertExtendNode(SSA::ssa_iterator ssa_iter, uint32_t src, uint32_t dst, int32_t bits);
};

#endif

#endif

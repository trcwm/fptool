/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

  Each multiplication with a CSD constant is
  expanded by introducting an addition or
  subtraction of shifted inputs for each digit.

  Author: Niels A. Moseley

*/

#ifndef csdmul_h
#define csdmul_h

#include <vector>
#include "ssapass.h"


class PassCSDMul : public SSAPass
{
protected:
    virtual void execute(SSAObject &ssa);

    /** Add a shift-and-add operation to the SSA list.
        x_idx is the index of the non-CSD variable.
        y_idx is the index of the result variable.
    */
    ssa_iterator shiftAndAdd(SSAObject &ssa, ssa_iterator where, const csd_t &csd, uint32_t x_idx, uint32_t y_idx);

};


#endif

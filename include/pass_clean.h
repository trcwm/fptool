/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Clean the SSA list

  1) remove re-interpret nodes
  2) remove superluous assignment nodes

*/


#ifndef clean_h
#define clean_h

#include <vector>
#include "ssapass.h"


class PassClean : public SSAPass
{
protected:
    virtual void execute(SSAObject &ssa);

    /** substitute op1 with op2 in SSA list*/
    void substitute(SSAObject &ssa, operandIndex idx1, operandIndex idx2);
};


#endif

/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Expand add subtract operations.
                Basically, before adding/subtracting,
                the number of fractional bits must
                be the same, so the operand with
                the least number of fractional bits
                is extended with zero bits.

*/

#ifndef passaddsub_h
#define passaddsub_h

#include "ssapass.h"

class PassAddSub : public SSAPass
{
protected:
    virtual void execute();
};

#endif

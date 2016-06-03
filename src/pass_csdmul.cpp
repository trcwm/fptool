/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

*/


#include "pass_addsub.h"

#include "ssapass.h"
class PassCSDMul : public SSAPass
{
protected:
    virtual void execute();
};

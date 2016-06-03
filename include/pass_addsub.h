/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

*/


#include "ssapass.h"

class PassAddSub : public SSAPass
{
protected:
    virtual void execute();
};

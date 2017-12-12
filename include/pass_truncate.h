/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Expand truncate operations into
                OP_RemoveLSBs/OP_ExtendLSBs and
                OP_RemoveMSBs/OP_ExtendMSBs nodes.

  Author: Niels A. Moseley

*/

#ifndef passtruncate_h
#define passtruncate_h

#include "ssapass.h"

class PassTruncate : public SSAPass
{
protected:
    virtual void execute(SSAObject &ssa);
};

#endif

/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  A base class that defines an SSA
                compiler pass interface

*/

#ifndef ssapass_h
#define ssapass_h

#include <stdexcept>
#include <sstream>
#include "ssa.h"

/** Generic compiler pass interface that processes an SSA tree.
    Errors should be handled through std::runtime_error exceptions.
*/
class SSAPass
{
public:
    void process(SSAObject &ssa)
    {
        execute(ssa);
    }

protected:
    virtual void execute(SSAObject &ssa) = 0;

};

#endif



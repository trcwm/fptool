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

namespace SSA
{

/** Generic compiler pass interface that processes an SSA tree.
    Errors should be handled through std::runtime_error exceptions.
*/
class TransformPass
{
public:
    void process(Program &ssa)
    {
        execute(ssa);
    }

protected:
    virtual void execute(Program &ssa) = 0;

};

} // namespace

#endif



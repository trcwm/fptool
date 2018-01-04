/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Print a single static assignment SSA
                list.

  Author: Niels A. Moseley

*/

#ifndef ssaprint_h
#define ssaprint_h

#include <iostream>
#include "ssa.h"

namespace SSA
{

class Printer : public OperationVisitorBase
{
public:
    Printer(std::ostream &s) : m_s(s) {}

    /** print the SSA progam data to an output stream */
    static bool print(const Program &program, std::ostream &s);

    virtual bool visit(const OpAssign *node) override;
    virtual bool visit(const OpMul *node) override;
    virtual bool visit(const OpAdd *node) override;
    virtual bool visit(const OpSub *node) override;
    virtual bool visit(const OpTruncate *node) override;
    virtual bool visit(const OperationSingle *node) override;
    virtual bool visit(const OperationDual *node) override;

protected:
    std::ostream &m_s;
};

} // namespace

#endif

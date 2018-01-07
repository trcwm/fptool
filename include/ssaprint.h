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
    Printer(std::ostream &s, bool printLHSPrecision)
        : m_printLHSPrecision(printLHSPrecision),
          m_s(s) {}

    /** print the SSA progam data to an output stream */
    static bool print(const Program &program, std::ostream &s, bool printLHSPrecision = false);

    virtual bool visit(const OpAssign *node) override;
    virtual bool visit(const OpMul *node) override;
    virtual bool visit(const OpCSDMul *node) override;
    virtual bool visit(const OpAdd *node) override;
    virtual bool visit(const OpSub *node) override;
    virtual bool visit(const OpTruncate *node) override;
    virtual bool visit(const OpReinterpret *node) override;

    virtual bool visit(const OpExtendLSBs *node) override;
    virtual bool visit(const OpExtendMSBs *node) override;
    virtual bool visit(const OpRemoveLSBs *node) override;
    virtual bool visit(const OpRemoveMSBs *node) override;

    virtual bool visit(const OpPatchBlock *node) override;
    virtual bool visit(const OpNull *node) override;
    virtual bool visit(const OperationSingle *node) override;
    virtual bool visit(const OperationDual *node) override;

protected:
    bool m_printLHSPrecision;
    std::ostream &m_s;
};

} // namespace

#endif

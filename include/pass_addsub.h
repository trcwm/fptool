/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Expand add subtract operations.
                Basically, before adding/subtracting,
                the number of fractional bits must
                be the same, so the operand with
                the least number of fractional bits
                is extended with zero bits.

  Author: Niels A. Moseley

*/

#ifndef passaddsub_h
#define passaddsub_h

#include "ssapass.h"

namespace SSA {

class PassAddSub : public OperationVisitorBase
{
public:
    /** equalize the fractional bits of both arguments before
        each add/sub instruction.
    */
    static bool execute(Program &ssa);

    // supported nodes!
    virtual bool visit(const OpAssign *node) override { return true; }
    virtual bool visit(const OpMul *node) override  { return true; }
    virtual bool visit(const OpAdd *node) override;
    virtual bool visit(const OpSub *node) override;
    virtual bool visit(const OpTruncate *node) override { return true; }
    virtual bool visit(const OpReinterpret *node) override { return true; }
    virtual bool visit(const OpPatchBlock *node) override { return true; }

    virtual bool visit(const OpExtendLSBs *node) override { return true; }
    virtual bool visit(const OpExtendMSBs *node) override { return true; }
    virtual bool visit(const OpRemoveLSBs *node) override { return true; }
    virtual bool visit(const OpRemoveMSBs *node) override { return true; }

    // unsupported nodes!
    virtual bool visit(const OperationSingle *node) override { return false; }
    virtual bool visit(const OperationDual *node) override { return false; }

protected:
    PassAddSub(Program &ssa) : m_ssa(&ssa)
    {
    }

    /** replace the node in the program with a patch block */
    void patchNode(const OperationBase *node, SSA::OpPatchBlock *patch);

    Program *m_ssa;
};

} // namespace

#endif

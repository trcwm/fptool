/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Clean the SSA list

  1) remove re-interpret nodes
  2) remove superluous assignment nodes

  Author: Niels A. Moseley

*/


#ifndef clean_h
#define clean_h

#include "ssa.h"

namespace SSA {

class PassClean : public OperationVisitorBase
{
public:
    /** equalize the fractional bits of both arguments before
        each add/sub instruction.
    */
    static bool execute(Program &ssa);

    // supported nodes!
    virtual bool visit(const OpAssign *node) override;
    virtual bool visit(const OpReinterpret *node) override;
    virtual bool visit(const OpMul *node) override  { return true; }
    virtual bool visit(const OpAdd *node) override  { return true; }
    virtual bool visit(const OpSub *node) override  { return true; }
    virtual bool visit(const OpTruncate *node) override { return true; }
    virtual bool visit(const OpPatchBlock *node) override { return true; }
    virtual bool visit(const OpNull *node) override { return true; }

    virtual bool visit(const OpExtendLSBs *node) override { return true; }
    virtual bool visit(const OpExtendMSBs *node) override { return true; }
    virtual bool visit(const OpRemoveLSBs *node) override { return true; }
    virtual bool visit(const OpRemoveMSBs *node) override { return true; }

    // unsupported nodes!
    virtual bool visit(const OperationSingle *node) override { return false; }
    virtual bool visit(const OperationDual *node) override { return false; }

protected:
    /* hide constructor so use can't call it directly */
    PassClean(Program &ssa) : m_ssa(&ssa)
    {
    }

    /** substitute op1 with op2 in SSA list
    */
    void substituteOperands(const SharedOpPtr op1, SharedOpPtr op2);

    /** replace the node in the program with a null operation */
    void replaceWithNull(const OperationBase *node);

    Program *m_ssa;
};

} // namespace

#endif

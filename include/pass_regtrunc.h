/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Insert truncate nodes for register assignments

  Author: Niels A. Moseley

*/


#ifndef regtrunc_h
#define regtrunc_h

#include "ssa.h"

namespace SSA {

class PassRegTrunc : public OperationVisitorBase
{
public:
    /** Remove superfluous assignment nodes.
    */
    static bool execute(Program &ssa);

    // supported nodes!
    virtual bool visit(const OpAssign *node) override;
    virtual bool visit(const OpReinterpret *node) override { (void)node; return true; }
    virtual bool visit(const OpMul *node) override  { (void)node; return true; }
    virtual bool visit(const OpCSDMul *node) override  { (void)node; return true; }
    virtual bool visit(const OpAdd *node) override  { (void)node; return true; }
    virtual bool visit(const OpSub *node) override  { (void)node; return true; }
    virtual bool visit(const OpTruncate *node) override { (void)node; return true; }
    virtual bool visit(const OpNegate *node) override { (void)node; return true; }
    virtual bool visit(const OpPatchBlock *node) override { (void)node; return true; }
    virtual bool visit(const OpNull *node) override { (void)node; return true; }

    virtual bool visit(const OpExtendLSBs *node) override { (void)node; return true; }
    virtual bool visit(const OpExtendMSBs *node) override { (void)node; return true; }
    virtual bool visit(const OpRemoveLSBs *node) override { (void)node; return true; }
    virtual bool visit(const OpRemoveMSBs *node) override { (void)node; return true; }

    // unsupported nodes!
    virtual bool visit(const OperationSingle *node) override { (void)node; return false; }
    virtual bool visit(const OperationDual *node) override { (void)node; return false; }

protected:
    /* hide constructor so use can't call it directly */
    explicit PassRegTrunc(Program &ssa) : m_ssa(&ssa)
    {
    }

    void patchNode(const SSA::OperationBase *node, SSA::OpPatchBlock *patch);

#if 0
    /** substitute op1 with op2 in SSA list
    */
    void substituteOperands(const SharedOpPtr &op1, SharedOpPtr op2);

    /** replace the node in the program with a null operation */
    void replaceWithNull(const OperationBase *node);
#endif

    Program *m_ssa;
};

} // namespace

#endif

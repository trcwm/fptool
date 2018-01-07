/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Expand truncate operations into
                OP_RemoveLSBs/OP_ExtendLSBs and
                OP_RemoveMSBs/OP_ExtendMSBs nodes.

  Author: Niels A. Moseley

*/

#ifndef passtruncate_h
#define passtruncate_h

#include "ssa.h"

namespace SSA {

class PassTruncate : public OperationVisitorBase
{
public:
    /** expand truncate instruction into
        ExtendMSB/ExtendLSB/RemoveMSB/RemoveLSB
        instructions.
    */
    static bool execute(Program &ssa);

    // supported nodes!
    virtual bool visit(const OpAssign *node) override  { (void)node; return true; }
    virtual bool visit(const OpReinterpret *node) override  { (void)node; return true; }
    virtual bool visit(const OpMul *node) override  { (void)node; return true; }
    virtual bool visit(const OpCSDMul *node) override  { (void)node; return true; }
    virtual bool visit(const OpAdd *node) override  { (void)node; return true; }
    virtual bool visit(const OpSub *node) override  { (void)node; return true; }
    virtual bool visit(const OpTruncate *node) override;
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
    PassTruncate(Program &ssa) : m_ssa(&ssa)
    {
    }

    /** replace the node in the program with a patch block */
    void patchNode(const SSA::OperationBase *node, SSA::OpPatchBlock *patch);

    Program *m_ssa;
};

}   // namespace

#endif

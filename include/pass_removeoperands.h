/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Clean the operand list

  1) Removes unused operands from the operand list.

  Author: Niels A. Moseley

*/


#ifndef removeoperands_h
#define removeoperands_h

#include "ssa.h"

namespace SSA {

class PassRemoveOperands : public OperationVisitorBase
{
public:
    /** remove unused operands from the operand list
    */
    static bool execute(Program &ssa);

    // supported nodes!
    virtual bool visit(const OpAssign *node) override;
    virtual bool visit(const OpAdd *node) override;
    virtual bool visit(const OpSub *node) override;
    virtual bool visit(const OpMul *node) override;
    virtual bool visit(const OpNegate *node) override;
    virtual bool visit(const OpExtendLSBs *node) override;
    virtual bool visit(const OpExtendMSBs *node) override;
    virtual bool visit(const OpRemoveLSBs *node) override;
    virtual bool visit(const OpRemoveMSBs *node) override;

    // unsupported nodes!
    virtual bool visit(const OperationSingle *node) override { (void)node; return false; }
    virtual bool visit(const OperationDual *node) override { (void)node; return false; }
    virtual bool visit(const OpCSDMul *node) override  { (void)node; return false; }
    virtual bool visit(const OpTruncate *node) override { (void)node; return false; }
    virtual bool visit(const OpNull *node) override { (void)node; return false; }
    virtual bool visit(const OpPatchBlock *node) override { (void)node; return false; }
    virtual bool visit(const OpReinterpret *node) override { (void)node; return false; }

protected:
    /* hide constructor so use can't call it directly */
    PassRemoveOperands(Program &ssa) : m_ssa(&ssa)
    {
    }

    Program *m_ssa;
};

} // namespace

#endif

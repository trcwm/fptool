/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

  Each multiplication with a CSD constant is
  expanded by introducting an addition or
  subtraction of shifted inputs for each digit.

  Author: Niels A. Moseley

*/

#ifndef csdmul_h
#define csdmul_h

#include <vector>
#include "ssa.h"

namespace SSA {

class PassCSDMul : public OperationVisitorBase
{
public:
    /** replace all CSD multiplications by shift-and-add instructions */
    static bool execute(Program &ssa);

    // supported nodes!
    virtual bool visit(const OpAssign *node) override { return true; }
    virtual bool visit(const OpMul *node) override;
    virtual bool visit(const OpAdd *node) override { return true; }
    virtual bool visit(const OpSub *node) override { return true; }
    virtual bool visit(const OpTruncate *node) override { return true; }
    virtual bool visit(const OpReinterpret *node) override { return true; }
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
    PassCSDMul(Program &ssa) : m_ssa(&ssa)
    {
    }

    /** expand CSD multiplication: produce instructions and operands
        that replace the original y := c*x instruction.

        @param[in] csd the constant expressed in canonical signed digit representation.
        @param[in] input pointer to the input operand.
        @param[in] output pointer to the output operand.
        @param[out] patch a patch block that will receive the replacement instructions.
        @param[out] operands a list of additional operands used by the new instructions.
    */
    void expandCSD(const csd_t &csd, const SharedOpPtr input, const SharedOpPtr output,
                   SSA::OpPatchBlock *patch,
                   std::list<SharedOpPtr> &operands);

    /** replace the node in the program with a patch block */
    void patchNode(const OperationBase *node, SSA::OpPatchBlock *patch);

    Program *m_ssa;
};

} // namespace

#endif

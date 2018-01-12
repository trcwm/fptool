/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  VHDL code generator that generates
                code using REAL data types for
                model verification.

                It accepts the following nodes:
                * ADD,SUB,MUL,CSDMUL, ASSIGN, NEGATE and TRUNCATE.

                As such, it can only be run on the SSA
                that is generated before _any_
                transforms/passes have been applied!

*/

#ifndef vhdlrealgen_h
#define vhdlrealgen_h

#include <iostream>
#include "ssa.h"

namespace SSA {

class VHDLRealGen : public OperationVisitorBase
{
public:
    static bool generateCode(std::ostream &os, Program &ssa)
    {
        VHDLRealGen generator(os, ssa);
        return generator.execute();
    }

    // supported nodes!
    virtual bool visit(const OpAssign *node) override;
    virtual bool visit(const OpMul *node) override;
    virtual bool visit(const OpAdd *node) override;
    virtual bool visit(const OpSub *node) override;
    virtual bool visit(const OpNegate *node) override;
    virtual bool visit(const OpCSDMul *node) override;
    virtual bool visit(const OpTruncate *node) override;

    // unsupported nodes!
    virtual bool visit(const OpExtendLSBs *node) override { (void)node; return false; }
    virtual bool visit(const OpExtendMSBs *node) override { (void)node; return false; }
    virtual bool visit(const OpRemoveLSBs *node) override { (void)node; return false; }
    virtual bool visit(const OpRemoveMSBs *node) override { (void)node; return false; }
    virtual bool visit(const OpNull *node) override { (void)node; return false; }
    virtual bool visit(const OperationSingle *node) override { (void)node; return false; }
    virtual bool visit(const OperationDual *node) override { (void)node; return false; }
    virtual bool visit(const OpPatchBlock *node) override { (void)node; return false; }
    virtual bool visit(const OpReinterpret *node) override  { (void)node; return false; }

protected:
    VHDLRealGen(std::ostream &os, Program &ssa);

    bool execute();
    void genProcessHeader(uint32_t indent);
    void genIndent(uint32_t indent);

    Program         *m_ssa;
    std::ostream    &m_os;
    uint32_t        m_indent;
    std::string     m_prolog;
    std::string     m_epilog;
};

} // end namespace

#endif

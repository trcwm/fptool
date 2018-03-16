/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  VHDL code generator

*/

#ifndef vhdlcodegen_h
#define vhdlcodegen_h

#include <iostream>
#include "ssa.h"

namespace SSA {

class VHDLCodeGen : public OperationVisitorBase
{
public:
    //VHDLCodeGen(std::ostream &os, Program &ssa) {}
    static bool generateCode(std::ostream &os, Program &ssa, bool genTestbench = false)
    {
        VHDLCodeGen generator(os, ssa, genTestbench);
        return generator.execute();
    }

    // supported nodes!
    virtual bool visit(const OpAssign *node) override;
    virtual bool visit(const OpMul *node) override;
    virtual bool visit(const OpAdd *node) override;
    virtual bool visit(const OpSub *node) override;
    virtual bool visit(const OpNegate *node) override;
    virtual bool visit(const OpNull *node) override;

    virtual bool visit(const OpExtendLSBs *node) override;
    virtual bool visit(const OpExtendMSBs *node) override;
    virtual bool visit(const OpRemoveLSBs *node) override;
    virtual bool visit(const OpRemoveMSBs *node) override;

    virtual bool visit(const OpReinterpret *node) override;

    // unsupported nodes!
    virtual bool visit(const OperationSingle *node) override { (void)node; return false; }
    virtual bool visit(const OperationDual *node) override { (void)node; return false; }
    virtual bool visit(const OpPatchBlock *node) override { (void)node; return false; }
    virtual bool visit(const OpCSDMul *node) override  { (void)node; return false; }
    virtual bool visit(const OpTruncate *node) override { (void)node; return false; }


protected:
    VHDLCodeGen(std::ostream &os, Program &ssa, bool genTestbench);

    bool execute();
    void genProcessHeader(uint32_t indent);
    void genIndent(uint32_t indent);

    void genEntity();
    void genArchitectureSignals();
    void genClockedProcess();
    void genTestbenchHeader();
    void genTestbenchFooter();

    // return a VHDL compatible length Hex literal
    std::string chopHexString(const std::string &hex, int32_t intBits, int32_t fracBits);

    Program         *m_ssa;
    std::ostream    &m_os;
    uint32_t        m_indent;
    std::string     m_prolog;
    std::string     m_epilog;
    bool            m_genTestbench;
};

} // end namespace

#endif

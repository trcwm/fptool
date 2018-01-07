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
    VHDLCodeGen(std::ostream &os, Program &ssa) {}

    // supported nodes!
    virtual bool visit(const OpAssign *node) override  { (void)node; return true; }
    virtual bool visit(const OpReinterpret *node) override  { (void)node; return true; }
    virtual bool visit(const OpMul *node) override  { (void)node; return true; }
    virtual bool visit(const OpCSDMul *node) override  { (void)node; return true; }
    virtual bool visit(const OpAdd *node) override  { (void)node; return true; }
    virtual bool visit(const OpSub *node) override  { (void)node; return true; }
    virtual bool visit(const OpTruncate *node) override { (void)node; return true; }
    virtual bool visit(const OpPatchBlock *node) override { (void)node; return true; }
    virtual bool visit(const OpNull *node) override { (void)node; return true; }

    virtual bool visit(const OpExtendLSBs *node) override { (void)node; return true; }
    virtual bool visit(const OpExtendMSBs *node) override { (void)node; return true; }
    virtual bool visit(const OpRemoveLSBs *node) override { (void)node; return true; }
    virtual bool visit(const OpRemoveMSBs *node) override { (void)node; return true; }

    // unsupported nodes!
    virtual bool visit(const OperationSingle *node) override { (void)node; return false; }
    virtual bool visit(const OperationDual *node) override { (void)node; return false; }
};

#if 0
class VHDLCodeGen : public OperationVisitorBase
{
public:
    VHDLCodeGen();

    void execute(std::ostream &os, Program &ssa);

    void setProlog(const std::string &s)
    {
        m_prolog = s;
    }

    void setEpilog(const std::string &s)
    {
        m_epilog = s;
    }

    // supported nodes!
    virtual bool visit(const OpAssign *node) override  { (void)node; return true; }
    virtual bool visit(const OpReinterpret *node) override  { (void)node; return true; }
    virtual bool visit(const OpMul *node) override  { (void)node; return true; }
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
    VHDLCodeGen(std::ostream &_os, Program &ssa)
        : m_os(_os), m_ssa(&ssa)
    {

    }

    /** generate left hand side of a statement */
    void genLHS(std::ostream &os, operand_t op, uint32_t indent);

    /** generate signals and variables etc. */
    void genProcessHeader(const SSAObject &ssa, std::ostream &os, uint32_t indent);

    /** generate indentation */
    void genIndent(std::ostream &os, uint32_t indent);

    /** extend MSBs of a signal by replicating the sign bit */
    void extendMSBs(std::ostream &os, std::string name, uint32_t bits);

    /** extend LSBs of a signal by adding zeroes */
    void extendLSBs(std::ostream &os, const std::string &name, uint32_t bits);

    /** generate a TO_SIGNED() argument in case of an integer literal */
    operand_t handleLiteralInt(operand_t &op);

    Program      *m_ssa;
    std::ostream &m_os;
    std::string  m_prolog;
    std::string  m_epilog;
};

#endif

} // end namespace

#endif

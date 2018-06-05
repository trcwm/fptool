/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Visitor base class

  Author: Niels A. Moseley



*/

#ifndef astvisitor_h
#define astvisitor_h

#include <ostream>
#include <stdint.h>
#include "astnode.h"

namespace AST
{

class DumpVisitor : public ASTVisitorBase
{
public:
    /** create an object to dump the AST to a stream */
    explicit DumpVisitor(std::ostream &os)
        : m_depth(0),
          m_os(os)
    {}

    /** dump a specific node */
    //virtual void visit(const Identifier *node);
    virtual void visit(const IntegerConstant *node) override;
    virtual void visit(const CSDDeclaration *node) override;
    virtual void visit(const RegDeclaration *node) override;
    virtual void visit(const Statements *node) override;
    virtual void visit(const InputDeclaration *node) override;
    virtual void visit(const PrecisionModifier *node) override;
    virtual void visit(const Assignment *node) override;
    virtual void visit(const Operation2 *node) override;
    virtual void visit(const Operation1 *node) override;

    virtual void visit(const InputVariable *node) override;
    virtual void visit(const OutputVariable *node) override;
    virtual void visit(const CSDConstant *node) override;
    virtual void visit(const Register *node) override;

protected:
    void doIndent();

    uint32_t     m_depth;
    std::ostream &m_os;
};


} // AST namespace




#endif

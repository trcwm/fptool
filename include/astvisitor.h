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
    virtual void visit(const Identifier *node);
    virtual void visit(const IntegerConstant *node);
    virtual void visit(const CSDDeclaration *node);
    virtual void visit(const RegDeclaration *node);
    virtual void visit(const Statements *node);
    virtual void visit(const InputDeclaration *node);
    virtual void visit(const PrecisionModifier *node);
    virtual void visit(const Assignment *node);
    virtual void visit(const Operation2 *node);
    virtual void visit(const Operation1 *node);

protected:
    void doIndent();

    uint32_t     m_depth;
    std::ostream &m_os;
};


} // AST namespace




#endif

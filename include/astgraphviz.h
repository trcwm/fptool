/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  A pass to produce a Graphviz output file
                for debugging.

  Author: Niels A. Moseley

*/

#ifndef pass_graphviz_h
#define pass_graphviz_h

#include <string>
#include "astnode.h"
#include "astvisitor.h"

class AST2Graphviz : public AST::VisitorBase
{
public:
    /** create an object to dump the AST to a stream */
    AST2Graphviz(std::ostream &os)
        : m_count(0),
          m_os(os)
    {}

    void writeProlog();
    void addStatement(const ASTNode *node) {};
    void writeEpilog();

    //void visit(const ASTNode *node) override;
    virtual void visit(const AST::Identifier *node) override;
    virtual void visit(const AST::IntegerConstant *node) override;
    virtual void visit(const AST::CSDDeclaration *node) override;
    virtual void visit(const AST::Statements *node) override;
    virtual void visit(const AST::InputDeclaration *node) override;
    virtual void visit(const AST::PrecisionModifier *node) override;
    virtual void visit(const AST::Assignment *node) override;

protected:

    int32_t m_count;
    std::ostream &m_os;
};

#endif

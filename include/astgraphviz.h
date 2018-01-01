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
    AST2Graphviz(std::ostream &os, bool noInputs=false)
        : m_noInputs(noInputs),
          m_count(0),
          m_os(os)
    {}

    void writeProlog();
    void addStatement(ASTNode *node);
    void writeEpilog();

    virtual void visit(const AST::Identifier *node) override;
    virtual void visit(const AST::IntegerConstant *node) override;
    virtual void visit(const AST::CSDDeclaration *node) override;
    virtual void visit(const AST::Statements *node) override;
    virtual void visit(const AST::InputDeclaration *node) override;
    virtual void visit(const AST::PrecisionModifier *node) override;
    virtual void visit(const AST::Assignment *node) override;
    virtual void visit(const AST::Operation2 *node) override;
    virtual void visit(const AST::Operation1 *node) override;

protected:
    bool m_noInputs;
    int32_t m_count;
    std::ostream &m_os;
};

#endif

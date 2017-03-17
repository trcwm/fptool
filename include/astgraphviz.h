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

class AST2Graphviz : public ASTVisitorBase
{
public:
    /** create an object to dump the AST to a stream */
    AST2Graphviz(std::ostream &os)
        : m_os(os),
          m_count(0)
    {}

    void writeProlog();
    void addStatement(const ASTNode *node);
    void writeEpilog();

    void visit(const ASTNode *node) override;
protected:
    int32_t m_count;
    std::ostream &m_os;
};

#endif

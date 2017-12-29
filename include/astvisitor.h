/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Visitor base class

  Author: Niels A. Moseley



*/

#ifndef astvisitor_h
#define astvisitor_h

#include <ostream>
#include <stdint.h>

class ASTNode; // pre-declaration

/** Visitor class for ASTNodes
    See: "Design Patterns: Elements of Reusable Object-Oriented Software"
*/
class ASTVisitorBase
{
public:
    virtual void visit(const ASTNode *node) = 0;
};


class ASTDumpVisitor : public ASTVisitorBase
{
public:
    /** create an object to dump the AST to a stream */
    ASTDumpVisitor(std::ostream &os)
        : m_os(os),
          m_depth(0)
    {}

    /** dump a specific node */
    virtual void visit(const ASTNode *node) override;

protected:
    uint32_t     m_depth;
    std::ostream &m_os;
};

#endif

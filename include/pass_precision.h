/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

  Set the Q(n,m) precisions of each node based on
  min/max numbers and lock the precision of the
  output variables.

  Author: Niels A. Moseley

*/

#ifndef passprecision_h
#define passprecision_h

#include <ostream>
#include <stdint.h>
#include <list>
#include "astnode.h"
#include "identdb.h"

namespace AST
{

class PassPrecision : public ASTVisitorBase
{
public:
    /** create an object to dump the AST to a stream */
    explicit PassPrecision(SymbolTable &symTable);

    /** process the AST tree */
    bool processAST(AST::ASTNodeBase *head);

    /** dump a specific node */
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
    struct precision_t
    {
        int32_t m_intBits;
        int32_t m_fracBits;
        fplib::SFix m_min;
        fplib::SFix m_max;
    };

    std::list<precision_t>  m_precisionStack;
    SymbolTable *m_symTable;
};

} // namespace AST

#endif


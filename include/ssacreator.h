/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Create a single-static assignment data structure
                from tha abstract syntax tree.

  Author: Niels A. Moseley

*/


#ifndef ssacreator_h
#define ssacreator_h

#include <string>
#include <iostream>
#include <stdexcept>

#include "astnode.h"
#include "astvisitor.h"
#include "ssa.h"
#include "identdb.h"

namespace SSA
{

class Creator : public AST::ASTVisitorBase
{
public:
    Creator();
    virtual ~Creator();

    /** create a single-static assignment list of statements,
        based on the abstract syntax tree and the symbol table. */
    bool process(const AST::Statements &statements,
                 const SymbolTable &symbols,
                 SSA::Program &ssa);

    virtual void visit(const AST::Identifier *node) override;
    virtual void visit(const AST::IntegerConstant *node) override;
    virtual void visit(const AST::CSDDeclaration *node) override;
    virtual void visit(const AST::RegDeclaration *node) override;
    virtual void visit(const AST::Statements *node) override;
    virtual void visit(const AST::InputDeclaration *node) override;
    virtual void visit(const AST::PrecisionModifier *node) override;
    virtual void visit(const AST::Assignment *node) override;
    virtual void visit(const AST::Operation2 *node) override;
    virtual void visit(const AST::Operation1 *node) override;

    std::string getLastError() const
    {
        return m_lastError;
    }

protected:
    /** push an operand onto the operand stack */
    void PushOperand(SharedOpPtr operand);
    SharedOpPtr PopOperand();

    /** emit an error in human readable form */
    void error(const std::string &errorstr)
    {
        //std::cout << errorstr << std::endl;
        m_lastError = errorstr;
        throw std::runtime_error(errorstr);
    }

    SSA::Program                *m_ssa;         ///< SSA program statements
    std::string                 m_lastError;    ///< last generated error
    std::list<SharedOpPtr>      m_opStack;      ///< operand stack
    const SymbolTable               *m_identDB;     ///< identifier database
};

} // namespace

#endif

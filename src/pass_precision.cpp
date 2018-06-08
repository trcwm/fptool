/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  CSD multiplication expander SSA pass

  Set the Q(n,m) precisions of each node based on
  min/max numbers and lock the precision of the
  output variables.

  Author: Niels A. Moseley 2018
          www.moseleyinstruments.com

*/

#include "pass_precision.h"

#include <memory>
#include <iostream>
#include <sstream>
#include "logging.h"
#include "csd.h"
#include "ssaprint.h"
#include "pass_csdmul.h"

using namespace AST;

AST::PassPrecision::PassPrecision(SymbolTable &symTable) : m_symTable(&symTable)
{
}

bool AST::PassPrecision::processAST(AST::ASTNodeBase *head)
{
    m_precisionStack.clear();
    if (head != NULL)
    {
        head->accept(this);
        return true;
    }
    return false;
}

/** dump a specific node */
void PassPrecision::visit(const IntegerConstant *node)
{
    //TODO: push precision onto the stack
}

void PassPrecision::visit(const CSDDeclaration *node)
{
    // dp nothing..
}

void PassPrecision::visit(const RegDeclaration *node)
{
    // do nothing..
}

void PassPrecision::visit(const Statements *node)
{
    // iterate over statements
    for(auto statement : node->m_statements)
    {
        statement->accept(this);
    }
}

void PassPrecision::visit(const InputDeclaration *node)
{
    // do nothing..
}

void PassPrecision::visit(const PrecisionModifier *node)
{
    //evaluate the argument tree..
    node->m_argNode->accept(this);

    // pop previous precision off the stack
    // and push the new one

    m_precisionStack.pop_back();
    // TODO: determine new min,max
    precision_t newPrecision;
    newPrecision.m_intBits = node->m_intBits;
    newPrecision.m_fracBits = node->m_fracBits;
    m_precisionStack.push_back(newPrecision);
}

void PassPrecision::visit(const Assignment *node)
{
    node->m_expr->accept(this); // evaluate expression first

    precision_t p = m_precisionStack.back();
    SymbolInfo *sinfo = m_symTable->getIdentifiedByName(node->m_identName);
    if (sinfo != NULL)
    {
        sinfo->m_intBits = p.m_intBits;
        sinfo->m_fracBits = p.m_fracBits;
        sinfo->m_min = p.m_min;
        sinfo->m_max = p.m_max;
    }
    m_precisionStack.pop_back();
}

void PassPrecision::visit(const Operation2 *node)
{
    // FIXME: actually base the resulting Q(n,m)
    // on the max and min range..
    //

    node->m_left->accept(this);
    node->m_right->accept(this);

    precision_t p2 = m_precisionStack.back();   // right node
    m_precisionStack.pop_back();
    precision_t p1 = m_precisionStack.back();   // left node
    m_precisionStack.pop_back();

    precision_t newp;
    switch(node->m_nodeType)
    {
    case Operation2::NodeSub:
    case Operation2::NodeAdd:
        newp.m_intBits  = std::max(p1.m_intBits, p2.m_intBits)+1;
        newp.m_fracBits = std::max(p1.m_fracBits, p2.m_fracBits);
        break;
    case Operation2::NodeMul:
        newp.m_intBits  = p1.m_intBits + p2.m_intBits - 1;
        newp.m_fracBits = p1.m_fracBits + p2.m_fracBits;
        break;
    case Operation2::NodeDiv:
        // result = p1/p2;
        //
        // Q(n1,m1)/Q(n2,m2) -> Q(n1+m2-1,n2+m1)
        newp.m_intBits  = p1.m_intBits + p2.m_fracBits - 1;
        newp.m_fracBits = p2.m_intBits + p1.m_fracBits;
        break;
    default:
        // TODO: we should generate an error here.
        break;
    }
    m_precisionStack.push_back(newp);
}

void PassPrecision::visit(const Operation1 *node)
{
    // FIXME: actually base the resulting Q(n,m)
    // on the max and min range..
    //
    node->m_expr->accept(this);

    precision_t pexpr = m_precisionStack.back();   // expr node
    m_precisionStack.pop_back();

    precision_t newp;
    switch(node->m_nodeType)
    {
    case Operation1::NodeUnaryMinus:
        newp.m_intBits = pexpr.m_intBits;
        newp.m_fracBits = pexpr.m_fracBits;
        break;
    default:
        throw std::runtime_error("Unknown node type");
        // error!
        break;
    }
    m_precisionStack.push_back(newp);
}

void PassPrecision::visit(const InputVariable *node)
{
    // use an input var
    // so push the precision onto the stack

    SymbolInfo *sinfo = m_symTable->getIdentifiedByName(node->m_name);
    if (sinfo != NULL)
    {
        if (sinfo->m_type != SymbolInfo::T_INPUT)
        {
            throw std::runtime_error("Symbol is not an input variable");
        }
        precision_t p;
        p.m_intBits = sinfo->m_intBits;
        p.m_fracBits = sinfo->m_fracBits;
        p.m_min = sinfo->m_min;
        p.m_max = sinfo->m_max;
        m_precisionStack.push_back(p);
    }
    else
    {
        throw std::runtime_error("Cannot find input symbol in symbol table");
        //TODO: signal an error!
    }
}

void PassPrecision::visit(const OutputVariable *node)
{
    SymbolInfo *sinfo = m_symTable->getIdentifiedByName(node->m_name);
    if (sinfo != NULL)
    {
        if (sinfo->m_type != SymbolInfo::T_OUTPUT)
        {
            throw std::runtime_error("Symbol is not an output variable");
        }
        precision_t p;
        p.m_intBits = sinfo->m_intBits;
        p.m_fracBits = sinfo->m_fracBits;
        p.m_min = sinfo->m_min;
        p.m_max = sinfo->m_max;
        m_precisionStack.push_back(p);
    }
    else
    {
        throw std::runtime_error("Cannot find CSD symbol in symbol table");
        //TODO: signal an error!
    }

    // use of an output variable is not permitted!
    //throw std::runtime_error("Use of an output variable is not permitted");
}

void PassPrecision::visit(const CSDConstant *node)
{
    // push precision of CSD onto the precision stack
    SymbolInfo *sinfo = m_symTable->getIdentifiedByName(node->m_name);
    if (sinfo != NULL)
    {
        if (sinfo->m_type != SymbolInfo::T_CSD)
        {
            throw std::runtime_error("Symbol is not a CSD");
        }

        precision_t p;
        p.m_intBits = sinfo->m_intBits;
        p.m_fracBits = sinfo->m_fracBits;
        p.m_min = sinfo->m_min;
        p.m_max = sinfo->m_max;
        m_precisionStack.push_back(p);
    }
    else
    {
        throw std::runtime_error("Cannot find CSD symbol in symbol table");
        //TODO: signal an error!
    }
}

void PassPrecision::visit(const Register *node)
{
    // push precision of register onto the precision stack
    SymbolInfo *sinfo = m_symTable->getIdentifiedByName(node->m_name);
    if (sinfo != NULL)
    {
        if (sinfo->m_type != SymbolInfo::T_REG)
        {
            throw std::runtime_error("Symbol is not a register");
        }

        precision_t p;
        p.m_intBits = sinfo->m_intBits;
        p.m_fracBits = sinfo->m_fracBits;
        p.m_min = sinfo->m_min;
        p.m_max = sinfo->m_max;
        m_precisionStack.push_back(p);
    }
    else
    {
        throw std::runtime_error("Cannot find register symbol in symbol table");
        //TODO: signal an error!
    }
}


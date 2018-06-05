/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Visitor base class

  Author: Niels A. Moseley

*/

#include "astnode.h"
#include "astvisitor.h"

void AST::DumpVisitor::doIndent()
{
    for(uint32_t i=0; i<m_depth; i++)
        m_os << "  ";
}

#if 0
void AST::DumpVisitor::visit(const Identifier *node)
{
    doIndent();
    m_os << node->m_identName.c_str() << "\n";
}
#endif

void AST::DumpVisitor::visit(const IntegerConstant *node)
{
    doIndent();
    m_os << node->m_value << "\n";
}

void AST::DumpVisitor::visit(const CSDDeclaration *node)
{
    doIndent();
    m_os << "CSD " << node->m_identName << " := " << node->m_csd.value << " [ ";
    for(csdigit_t d : node->m_csd.digits)
    {
        if (d.sign > 0)
        {
            m_os << "+2^" << d.power;
        }
        else
        {
            m_os << "-2^" << d.power;
        }
        m_os << " ";
    }
    m_os << "]\n";
}

void AST::DumpVisitor::visit(const Statements *node)
{
    m_os << "Statements";
    for(ASTNodeBase *stmt : node->m_statements)
    {
        if (stmt != NULL)
        {
            stmt->accept(this);
        }
    }
}

void AST::DumpVisitor::visit(const InputDeclaration *node)
{
    doIndent();
    m_os << "Input " << node->m_identName.c_str() << " Q(" << node->m_intBits << "," << node->m_fracBits << ")\n";
}

void AST::DumpVisitor::visit(const RegDeclaration *node)
{
    doIndent();
    m_os << "Register " << node->m_identName.c_str() << " Q(" << node->m_intBits << "," << node->m_fracBits << ")\n";
}

void AST::DumpVisitor::visit(const PrecisionModifier *node)
{
    if (node->m_argNode!= NULL)
    {
        m_depth++;
        node->m_argNode->accept(this);
        m_depth--;
    }

    doIndent();
    switch(node->m_nodeType)
    {
    case PrecisionModifier::NodeTruncate:
        m_os << "Truncate to Q(" << node->m_intBits << "," << node->m_fracBits << ")\n";
        break;
    default:
        m_os << "ERROR: DumpVisitor::visit PrecisionModifier unhandled node type!";
        break;
    }
}

void AST::DumpVisitor::visit(const Assignment *node)
{
    if (node->m_expr != NULL)
    {
        m_depth++;
        node->m_expr->accept(this);
        m_depth--;
    }

    doIndent();
    m_os << node->m_identName.c_str() << " := \n";
}

void AST::DumpVisitor::visit(const Operation2 *node)
{
    if (node->m_left != NULL)
    {
        m_depth++;
        node->m_left->accept(this);
        m_depth--;
    }

    if (node->m_right != NULL)
    {
        m_depth++;
        node->m_right->accept(this);
        m_depth--;
    }

    doIndent();
    switch(node->m_nodeType)
    {
    case Operation2::NodeAdd:
        m_os << " + \n";
        break;
    case Operation2::NodeSub:
        m_os << " - \n";
        break;
    case Operation2::NodeMul:
        m_os << " * \n";
        break;
    case Operation2::NodeDiv:
        m_os << " / \n";
        break;
    default:
        m_os << "ERROR: DumpVisitor::visit Operation2 unhandled node type!";
        break;
    };
}

void AST::DumpVisitor::visit(const Operation1 *node)
{

}

void AST::DumpVisitor::visit(const InputVariable *node)
{
    doIndent();
    m_os << node->m_name.c_str() << "\n";
}

void AST::DumpVisitor::visit(const OutputVariable *node)
{
    doIndent();
    m_os << node->m_name.c_str() << "\n";
}

void AST::DumpVisitor::visit(const CSDConstant *node)
{
    doIndent();
    m_os << node->m_name.c_str() << "\n";
}

void AST::DumpVisitor::visit(const Register *node)
{
    doIndent();
    m_os << node->m_name.c_str() << "\n";
}

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
void AST::DumpVisitor::visit(const ASTNode *node)
{
    if (node == NULL) return;

    if (node->left != 0)
    {
        m_depth++;
        node->left->accept(this);
        m_depth--;
    }
    if (node->right != 0)
    {
        m_depth++;
        node->right->accept(this);
        m_depth--;
    }

    // dump the current node
    // indent according to level
    for(uint32_t i=0; i<m_depth; i++)
        m_os << "  ";

    switch(node->type)
    {
    case ASTNode::NodeUnknown:
        m_os << "Unknown";
        break;
    case ASTNode::NodeAssign:
        //m_os << node->info.txt << " = ";
        break;
    case ASTNode::NodeInput:
        //m_os << "INPUT "<< node->info.txt << " Q(" << node->info.intBits << "," << node->info.fracBits << ")";
        break;
    case ASTNode::NodeCSD:
        //m_os << "CSD " << node->info.txt << " Q(" << node->info.csdFloat << "," << node->info.csdBits << ")";
        break;
    case ASTNode::NodeAdd:
        m_os << "+";
        break;
    case ASTNode::NodeSub:
        m_os << "-";
        break;
    case ASTNode::NodeMul:
        m_os << "*";
        break;
    case ASTNode::NodeUnaryMinus:
        m_os << "U-";
        break;
    case ASTNode::NodeIdent:
        //m_os << node->info.txt;
        break;
    case ASTNode::NodeInteger:
        //m_os << node->info.intVal;
        break;
    case ASTNode::NodeFloat:
        m_os << "FLOATVAL";
        //stream << floatVal;
        break;
    case ASTNode::NodeDiv:
        m_os << "/";
        break;
    case ASTNode::NodeTruncate:
        //m_os << "Truncate to Q(" << node->info.intBits << "," << node->info.fracBits << ")";
        break;
    default:
        m_os << "???";
        break;
    }
    m_os << "\n";
}
#endif


void AST::DumpVisitor::visit(const Identifier *node)
{
    doIndent();
    m_os << node->m_identName.c_str() << "\n";
}

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
    for(ASTNode *stmt : node->m_statements)
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

void AST::DumpVisitor::visit(const PrecisionModifier *node)
{
    if (node->right != NULL)
    {
        m_depth++;
        node->right->accept(this);
        m_depth--;
    }

    doIndent();
    switch(node->type)
    {
    case ASTNode::NodeTruncate:
        m_os << "Truncate to Q(" << node->m_intBits << "," << node->m_fracBits << ")\n";
        break;
    default:
        m_os << "ERROR: DumpVisitor::visit PrecisionModifier unhandled node type!";
        break;
    }
}

void AST::DumpVisitor::visit(const Assignment *node)
{
    if (node->right != NULL)
    {
        m_depth++;
        node->right->accept(this);
        m_depth--;
    }

    doIndent();
    m_os << node->m_identName.c_str() << " := \n";
}

void AST::DumpVisitor::visit(const Operation2 *node)
{
    if (node->left != NULL)
    {
        m_depth++;
        node->left->accept(this);
        m_depth--;
    }

    if (node->right != NULL)
    {
        m_depth++;
        node->right->accept(this);
        m_depth--;
    }

    doIndent();
    switch(node->type)
    {
    case ASTNode::NodeAdd:
        m_os << " + \n";
        break;
    case ASTNode::NodeSub:
        m_os << " - \n";
        break;
    case ASTNode::NodeMul:
        m_os << " * \n";
        break;
    case ASTNode::NodeDiv:
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

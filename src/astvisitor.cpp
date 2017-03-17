/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Visitor base class

  Author: Niels A. Moseley

*/

#include "astnode.h"
#include "astvisitor.h"

void ASTDumpVisitor::visit(const ASTNode *node)
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
        m_os << node->info.txt << " = ";
        break;
    case ASTNode::NodeInput:
        m_os << "INPUT Q(" << node->info.intBits << "," << node->info.fracBits << ")";
        break;
    case ASTNode::NodeCSD:
        m_os << "CSD (" << node->info.csdFloat << "," << node->info.csdBits << ")";
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
        m_os << node->info.txt;
        break;
    case ASTNode::NodeInteger:
        m_os << node->info.intVal;
        break;
    case ASTNode::NodeFloat:
        m_os << "FLOATVAL";
        //stream << floatVal;
        break;
    case ASTNode::NodeDiv:
        m_os << "/";
        break;
    default:
        m_os << "???";
        break;
    }
    m_os << "\n";
}

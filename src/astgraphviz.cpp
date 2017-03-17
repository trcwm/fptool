/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Output the AST as a Graphviz file
                for debugging.

  Author: Niels A. Moseley

*/

#include "astgraphviz.h"

void AST2Graphviz::writeProlog()
{
    // create the header
    m_os << "digraph BST {\n";
    m_os << "  node [fontname=\"Arial\"];\n";
}

void AST2Graphviz::writeEpilog()
{
    m_os << "}\n";
}

void AST2Graphviz::addStatement(const ASTNode *node)
{
    visit(node);
    m_count++;
}

void AST2Graphviz::visit(const ASTNode *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;
    int32_t leftNodeID = -1;
    int32_t rightNodeID = -1;

    if (node->left != 0)
    {
        m_count++;
        leftNodeID = m_count;
        node->left->accept(this);
    }
    if (node->right != 0)
    {
        m_count++;
        rightNodeID = m_count;
        node->right->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    switch(node->type)
    {
    case ASTNode::NodeUnknown:
        m_os << "Unknown";
        break;
    case ASTNode::NodeAssign:
        m_os << node->info.txt << " = ";
        break;
    case ASTNode::NodeInput:
        m_os << "INPUT " << node->info.txt << " Q(" << node->info.intBits << "," << node->info.fracBits << ")";
        break;
    case ASTNode::NodeCSD:
        m_os << "CSD " << node->info.txt << " (" << node->info.csdFloat << "," << node->info.csdBits << ")";
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
    m_os << "\"];\n";

    if (leftNodeID != -1)
    {
        m_os << thisNodeID << " -> " << leftNodeID << ";\n";
    }

    if (rightNodeID != -1)
    {
        m_os << thisNodeID << " -> " << rightNodeID << ";\n";
    }
}


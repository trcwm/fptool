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

void AST2Graphviz::addStatement(ASTNode *node)
{
    if (node != NULL)
    {
        node->accept(this);
        m_count++;
    }
}

void AST2Graphviz::visit(const AST::Identifier *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;

    m_os << thisNodeID << " [label=\"";
    m_os << node->m_identName.c_str();
    m_os << "\"];\n";
}

void AST2Graphviz::visit(const AST::IntegerConstant *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;

    m_os << thisNodeID << " [label=\"";
    m_os << node->m_value;
    m_os << "\"];\n";
}

void AST2Graphviz::visit(const AST::CSDDeclaration *node)
{
    if (node == 0) return;

    if (m_noInputs) return;

    int32_t thisNodeID = m_count;

    m_os << thisNodeID << " [label=\"";
    m_os << "CSD " << node->m_identName.c_str() << " " << node->m_csd.value;
    m_os << "\"];\n";
}

void AST2Graphviz::visit(const AST::Statements *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;

    // iterate over all child nodes
    std::vector<int32_t> m_cnodeNums; // child node indeces.
    for(ASTNode *cnode : node->m_statements)
    {
        m_count++;
        m_cnodeNums.push_back(m_count);
        cnode->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    m_os << "Statements";
    m_os << "\"];\n";

    // connect up all the child nodes to this node.
    for(int32_t num : m_cnodeNums)
    {
        m_os << thisNodeID << " -> " << num << " [dir=back];\n";
    }
}

void AST2Graphviz::visit(const AST::InputDeclaration *node)
{
    if (node == 0) return;

    if (m_noInputs) return;

    int32_t thisNodeID = m_count;

    m_os << thisNodeID << " [label=\"";
    m_os << "INPUT " << node->m_identName.c_str() << " Q(" << node->m_intBits << "," << node->m_fracBits << ")";
    m_os << "\"];\n";
}

void AST2Graphviz::visit(const AST::PrecisionModifier *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;
    //int32_t leftNodeID = -1;
    int32_t rightNodeID = -1;

    /*
    if (node->left != 0)
    {
        m_count++;
        leftNodeID = m_count;
        node->left->accept(this);
    }
    */

    if (node->right != 0)
    {
        m_count++;
        rightNodeID = m_count;
        node->right->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    switch(node->type)
    {
    case ASTNode::NodeTruncate:
        m_os << "TRUNC Q(" << node->m_intBits << "," << node->m_fracBits << ")";
        break;
    default:
        m_os << "INCORRECT TYPE";
        break;
    }
    m_os << "\"];\n";

    /*
    if (leftNodeID != -1)
    {
        m_os << thisNodeID << " -> " << leftNodeID << ";\n";
    }
    */

    if (rightNodeID != -1)
    {
        m_os << thisNodeID << " -> " << rightNodeID << " [dir=back];\n";
    }
}

void AST2Graphviz::visit(const AST::Assignment *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;
    //int32_t leftNodeID = -1;
    int32_t rightNodeID = -1;

    /*
    if (node->left != 0)
    {
        m_count++;
        leftNodeID = m_count;
        node->left->accept(this);
    }
    */

    if (node->right != 0)
    {
        m_count++;
        rightNodeID = m_count;
        node->right->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    m_os << node->m_identName.c_str() << " = ";
    m_os << "\"];\n";

    /*
    if (leftNodeID != -1)
    {
        m_os << thisNodeID << " -> " << leftNodeID << ";\n";
    }
    */

    if (rightNodeID != -1)
    {
        m_os << thisNodeID << " -> " << rightNodeID << " [dir=back];\n";
    }
}

void AST2Graphviz::visit(const AST::Operation2 *node)
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
    case ASTNode::NodeAdd:
        m_os << "+";
        break;
    case ASTNode::NodeSub:
        m_os << "-";
        break;
    case ASTNode::NodeMul:
        m_os << "*";
        break;
    case ASTNode::NodeDiv:
        m_os << "/";
        break;
    default:
        m_os << "INCORRECT TYPE";
        break;
    }
    m_os << "\"];\n";

    if (leftNodeID != -1)
    {
        m_os << thisNodeID << " -> " << leftNodeID << " [dir=back];\n";
    }

    if (rightNodeID != -1)
    {
        m_os << thisNodeID << " -> " << rightNodeID << " [dir=back];\n";
    }
}

void AST2Graphviz::visit(const AST::Operation1 *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;
    //int32_t leftNodeID = -1;
    int32_t rightNodeID = -1;

    /*
    if (node->left != 0)
    {
        m_count++;
        leftNodeID = m_count;
        node->left->accept(this);
    }
    */

    if (node->right != 0)
    {
        m_count++;
        rightNodeID = m_count;
        node->right->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    switch(node->type)
    {
    case ASTNode::NodeUnaryMinus:
        m_os << "U-";
        break;
    default:
        m_os << "INCORRECT TYPE";
        break;
    }
    m_os << "\"];\n";

    /*
    if (leftNodeID != -1)
    {
        m_os << thisNodeID << " -> " << leftNodeID << ";\n";
    }
    */

    if (rightNodeID != -1)
    {
        m_os << thisNodeID << " -> " << rightNodeID << " [dir=back];\n";
    }
}


#if 0
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
        //m_os << node->info.txt << " = ";
        break;
    case ASTNode::NodeInput:
        //m_os << "INPUT " << node->info.txt << " Q(" << node->info.intBits << "," << node->info.fracBits << ")";
        break;
    case ASTNode::NodeCSD:
        //m_os << "CSD " << node->info.txt << " (" << node->info.csdFloat << "," << node->info.csdBits << ")";
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

#endif

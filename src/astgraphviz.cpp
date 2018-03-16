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

void AST2Graphviz::visit(const AST::RegDeclaration *node)
{
    if (node == 0) return;

    if (m_noInputs) return;

    int32_t thisNodeID = m_count;

    m_os << thisNodeID << " [label=\"";
    m_os << "REG " << node->m_identName.c_str() << " Q(" << node->m_intBits << "," << node->m_fracBits << ")";
    m_os << "\"];\n";
}

void AST2Graphviz::visit(const AST::PrecisionModifier *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;
    int32_t argNodeID = -1;

    if (node->m_argNode != 0)
    {
        m_count++;
        argNodeID = m_count;
        node->m_argNode->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    switch(node->m_nodeType)
    {
    case AST::PrecisionModifier::NodeTruncate:
        m_os << "TRUNC Q(" << node->m_intBits << "," << node->m_fracBits << ")";
        break;
    default:
        m_os << "INCORRECT TYPE";
        break;
    }
    m_os << "\"];\n";

    if (argNodeID != -1)
    {
        m_os << thisNodeID << " -> " << argNodeID << " [dir=back];\n";
    }
}

void AST2Graphviz::visit(const AST::Assignment *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;
    int32_t exprNodeID = -1;

    if (node->m_expr != 0)
    {
        m_count++;
        exprNodeID = m_count;
        node->m_expr->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    m_os << node->m_identName.c_str() << " = ";
    m_os << "\"];\n";

    if (exprNodeID != -1)
    {
        m_os << thisNodeID << " -> " << exprNodeID << " [dir=back];\n";
    }
}

void AST2Graphviz::visit(const AST::Operation2 *node)
{
    if (node == 0) return;

    int32_t thisNodeID = m_count;
    int32_t leftNodeID = -1;
    int32_t rightNodeID = -1;

    if (node->m_left != 0)
    {
        m_count++;
        leftNodeID = m_count;
        node->m_left->accept(this);
    }
    if (node->m_right != 0)
    {
        m_count++;
        rightNodeID = m_count;
        node->m_right->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    switch(node->m_nodeType)
    {
    case AST::Operation2::NodeAdd:
        m_os << "+";
        break;
    case AST::Operation2::NodeSub:
        m_os << "-";
        break;
    case AST::Operation2::NodeMul:
        m_os << "*";
        break;
    case AST::Operation2::NodeDiv:
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
    int32_t exprNodeID = -1;

    if (node->m_expr != 0)
    {
        m_count++;
        exprNodeID = m_count;
        node->m_expr->accept(this);
    }

    m_os << thisNodeID << " [label=\"";
    switch(node->m_nodeType)
    {
    case AST::Operation1::NodeUnaryMinus:
        m_os << "U-";
        break;
    default:
        m_os << "INCORRECT TYPE";
        break;
    }
    m_os << "\"];\n";

    if (exprNodeID != -1)
    {
        m_os << thisNodeID << " -> " << exprNodeID << " [dir=back];\n";
    }
}


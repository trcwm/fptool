
#include "ssacreator.h"
#include <memory>

using namespace SSA;

Creator::Creator()
{

}

Creator::~Creator()
{

}

SharedOpPtr Creator::PopOperand()
{
    SharedOpPtr operand = m_opStack.back();
    m_opStack.pop_back();
    return operand;
}

void Creator::PushOperand(SharedOpPtr operand)
{
    m_opStack.push_back(operand);
}

bool Creator::process(AST::Statements &statements, SSA::Program &ssa)
{
    m_ssa = &ssa;

    for(ASTNode *node : statements.m_statements)
    {
        if (node != NULL)
        {
            node->accept(this);
        }
    }
    return true;
}


#if 0
if (node->left != 0)
{
    executeASTNode(node->left, ssa);
}

if (node->right != 0)
    executeASTNode(node->right, ssa);

operandIndex index;
switch(node->type)
{
default:
case ASTNode::NodeUnknown:
    return false;

// ************************************************
//  operations that only push things on the stack
// ************************************************
case ASTNode::NodeCSD:
    // CSD definition, no need to add to the stack
    // it's referenced by name
    ssa.addOperand(operand_t::TypeCSD, node->info);
    return true;
case ASTNode::NodeFloat:
    error("SSACreator - found an unsupported float literal!");
    return false;
case ASTNode::NodeInteger:
    // Literal integer, create an integer on the stack
    //
    index = ssa.addOperand(operand_t::TypeInteger, node->info);
    m_opStack.push_back(index);
    return true;
case ASTNode::NodeInput:
    // INPUT definition, no need to add to the stack
    // it's referenced by name
    ssa.addOperand(operand_t::TypeInput, node->info);
    return true;
case ASTNode::NodeIdent:
    // resolve the identifier by name
    if (!ssa.getOperandIndexByName(node->info.txt, index))
    {
        error("ASTNode::NodeIdent - identifier not found");
        return false;
    }
    m_opStack.push_back(index);
    return true;

// *************************************************
//  operations that pop and push stuff on the stack
// *************************************************
case ASTNode::NodeAssign:
{
    // assign to an output/register
    // sanity checking
    if (m_opStack.size() < 1)
    {
        error("ASTNode::NodeAssign - not enough operands on the stack");
        return false;
    }

    // one item at top of stack:
    //
    uint32_t arg1_idx = m_opStack.back();
    m_opStack.pop_back();

    // create an output variable
    operand_t rhs = ssa.getOperand(arg1_idx);
    node->info.intBits = rhs.info.intBits;
    node->info.fracBits = rhs.info.fracBits;
    index = ssa.addOperand(operand_t::TypeOutput, node->info);

    ssa.createAssignNode(ssa.end(), index, arg1_idx);
    return true;
}
case ASTNode::NodeAdd:
{
    // sanity checking
    if (m_opStack.size() < 2)
    {
        error("ASTNode::NodeAdd - not enough operands on the stack");
        // not enough operands!
        return false;
    }

    // two items at top of stack:
    //
    uint32_t arg2_idx = m_opStack.back();
    m_opStack.pop_back();
    uint32_t arg1_idx = m_opStack.back();
    m_opStack.pop_back();

    index = ssa.createAddNode(ssa.end(), arg1_idx, arg2_idx);
    m_opStack.push_back(index);
    return true;
}
case ASTNode::NodeSub:
{
    // sanity checking
    if (m_opStack.size() < 2)
    {
        error("ASTNode::NodeSub - not enough operands on the stack");
        // not enough operands!
        return false;
    }

    // two items at top of stack:
    //
    uint32_t arg2_idx = m_opStack.back();
    m_opStack.pop_back();
    uint32_t arg1_idx = m_opStack.back();
    m_opStack.pop_back();

    index = ssa.createSubNode(ssa.end(), arg1_idx, arg2_idx);
    m_opStack.push_back(index);
    return true;
}
case ASTNode::NodeMul:
{
    // sanity checking
    if (m_opStack.size() < 2)
    {
        error("NodeMul - not enough operands on the stack");
        // not enough operands!
        return false;
    }

    // two items at top of stack:
    //
    uint32_t arg2_idx = m_opStack.back();
    m_opStack.pop_back();
    uint32_t arg1_idx = m_opStack.back();
    m_opStack.pop_back();

    index = ssa.createMulNode(ssa.end(), arg1_idx, arg2_idx);
    m_opStack.push_back(index);

    return true;
}
case ASTNode::NodeDiv:
{
    // sanity checking
    if (m_opStack.size() < 2)
    {
        error("NodeDiv - not enough operands on the stack");
        // not enough operands!
        return false;
    }

    // two items at top of stack:
    //
    uint32_t arg2_idx = m_opStack.back();
    m_opStack.pop_back();
    uint32_t arg1_idx = m_opStack.back();
    m_opStack.pop_back();

    index = ssa.createDivNode(ssa.end(), arg1_idx, arg2_idx);
    m_opStack.push_back(index);

    return true;
}
case ASTNode::NodeUnaryMinus:
{
    // sanity checking
    if (m_opStack.size() < 1)
    {
        error("NodeUnaryMinus - not enough operands on the stack");
        // not enough operands!
        return false;
    }

    // one item at top of stack:
    //
    uint32_t arg1_idx = m_opStack.back();
    m_opStack.pop_back();

    operand_t rhs = ssa.getOperand(arg1_idx);
    node->info.intBits = rhs.info.intBits;
    node->info.fracBits = rhs.info.fracBits;

    index = ssa.createNegateNode(ssa.end(), arg1_idx);
    m_opStack.push_back(index);

    return true;
}
case ASTNode::NodeTruncate:
{
    // sanity checking
    if (m_opStack.size() < 1)
    {
        error("NodeTruncate - not enough operands on the stack");
        // not enough operands!
        return false;
    }

    // one item at top of stack:
    uint32_t arg1_idx = m_opStack.back();
    m_opStack.pop_back();

    index = ssa.createTruncateNode(ssa.end(), arg1_idx, node->info.intBits, node->info.fracBits);
    m_opStack.push_back(index);

    return true;
}
} // end switch
return true;
}
#endif

void Creator::visit(const AST::Assignment *node)
{
    // evaluate RHS expression
    if (node->m_expr != 0)
    {
        node->m_expr->accept(this);
    }

    // assign to an output/register
    // sanity checking
    if (m_opStack.size() < 1)
    {
        error("Creator::visit Assignment - not enough operands on the stack");
        return;
    }

    // pop expression argument item at top of stack:
    SharedOpPtr arg1 = PopOperand();

    SSA::SharedOpPtr result = std::make_shared<OutputOperand>();
    result->m_identName = node->m_identName;
    result->m_intBits   = arg1->m_intBits;
    result->m_fracBits  = arg1->m_fracBits;

    SSA::OpAssign *assign = new SSA::OpAssign(arg1, result);
    m_ssa->addStatement(assign);
    m_ssa->addOperand(result);
}

void Creator::visit(const AST::CSDDeclaration *node)
{
    // create an input variable
    std::shared_ptr<CSDOperand> csdop = std::make_shared<CSDOperand>();
    csdop->m_csd = node->m_csd;
    csdop->m_identName = node->m_identName;
    csdop->m_intBits = 0;
    csdop->m_fracBits = 0;

    m_ssa->addOperand(csdop);
}

void Creator::visit(const AST::Identifier *node)
{
    // lookup the identifier and store it in
    // the available variable list

    for(auto ptr : m_ssa->m_operands)
    {
        if (ptr->m_identName == node->m_identName)
        {
            PushOperand(ptr);
            return;
        }
    }
    // if we end up here, the identifier was not found
    // FIXME: return an error / throw exception
}


void Creator::visit(const AST::InputDeclaration *node)
{
    // create an input variable
    SSA::SharedOpPtr result = std::make_shared<InputOperand>();
    result->m_intBits   = node->m_intBits;
    result->m_fracBits  = node->m_fracBits;
    result->m_identName = node->m_identName;
    m_ssa->addOperand(result);
}


void Creator::visit(const AST::IntegerConstant *node)
{
    // not implemented yet
}


void Creator::visit(const AST::PrecisionModifier *node)
{
    node->m_argNode->accept(this);

    if (m_opStack.size() < 1)
    {
        error("Creator::visit PrecisionModifier - not enough operands on the stack");
        return;
    }

    // pop expression argument item at top of stack:
    SharedOpPtr arg1 = PopOperand();
    SSA::SharedOpPtr result;
    SSA::OperationSingle *statement;
    switch(node->m_nodeType)
    {
    case AST::PrecisionModifier::NodeTruncate:
        result = IntermediateOperand::createNewIntermediate();
        statement = new SSA::OpTruncate(arg1, result,
                                        node->m_intBits,
                                        node->m_fracBits);
        m_ssa->addStatement(statement);
        m_ssa->addOperand(result);
        PushOperand(result);
        break;
    default:
        error("Creator::visit PrecisionModified unsupported node");
        break;
    }
}


void Creator::visit(const AST::Statements *node)
{

}

void Creator::visit(const AST::Operation2 *node)
{
    node->m_left->accept(this);
    node->m_right->accept(this);

    if (m_opStack.size() < 2)
    {
        error("Creator::visit Operation2 - not enough operands on the stack");
        return;
    }

    // pop expression argument item at top of stack:
    SharedOpPtr arg2 = PopOperand();
    SharedOpPtr arg1 = PopOperand();

    SSA::SharedOpPtr result;
    SSA::OperationDual *statement;
    switch(node->m_nodeType)
    {
    case AST::Operation2::NodeAdd:
        result = IntermediateOperand::createNewIntermediate();
        statement = new SSA::OpAdd(arg1, arg2, result);
        m_ssa->addStatement(statement);
        m_ssa->addOperand(result);
        PushOperand(result);
        break;
    case AST::Operation2::NodeSub:
        result = IntermediateOperand::createNewIntermediate();
        statement = new SSA::OpSub(arg1, arg2, result);
        m_ssa->addStatement(statement);
        m_ssa->addOperand(result);
        PushOperand(result);
        break;
    case AST::Operation2::NodeMul:
        result = IntermediateOperand::createNewIntermediate();
        // create a CSDMul command if one of the arguments is
        // a CSD, otherwise create a regular MUL command.
        if (arg1->isCSD())
        {
            CSDOperand *csdop = dynamic_cast<CSDOperand *>(arg1.get());
            SSA::OpCSDMul* mulop = new SSA::OpCSDMul(arg2,
                                                     csdop->m_csd,
                                                     arg1->m_identName,
                                                     result);
            m_ssa->addStatement(mulop);
        }
        else if (arg2->isCSD())
        {
            CSDOperand *csdop = dynamic_cast<CSDOperand *>(arg2.get());
            SSA::OpCSDMul* mulop = new SSA::OpCSDMul(arg1,
                                                     csdop->m_csd,
                                                     arg2->m_identName,
                                                     result);
            m_ssa->addStatement(mulop);
        }
        else
        {
            statement = new SSA::OpMul(arg1, arg2, result);
            m_ssa->addStatement(statement);
        }

        m_ssa->addOperand(result);
        PushOperand(result);
        break;
    default:
        error("Creator::visit Operation2 unsupported node");
        break;
    }
}

void Creator::visit(const AST::Operation1 *node)
{
    node->m_expr->accept(this);

    if (m_opStack.size() < 1)
    {
        error("Creator::visit Operation1 - not enough operands on the stack");
        return;
    }

    // pop expression argument item at top of stack:
    SharedOpPtr arg1 = PopOperand();
    SSA::SharedOpPtr result;
    SSA::OperationSingle *statement;
    switch(node->m_nodeType)
    {
    case AST::Operation1::NodeUnaryMinus:
        result = IntermediateOperand::createNewIntermediate();
        statement = new SSA::OpNegate(arg1, result);
        m_ssa->addStatement(statement);
        m_ssa->addOperand(result);
        PushOperand(result);
        break;
    default:
        error("Creator::visit Operation1 unsupported node");
        break;
    }
}


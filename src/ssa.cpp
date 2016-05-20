/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Single static assignment intermediate representation
                for code generation.

  Author: Niels A. Moseley

*/

#include "ssa.h"
#include <math.h>
#include <sstream>

bool SSACreator::process(const statements_t &statements, ssaList_t &ssaList, ssaOperandList_t &ssaOperandList)
{
    m_lastError.clear();
    m_opStack.clear();
    m_ssaList = &ssaList;
    m_operandList = &ssaOperandList;

    size_t N = statements.size();
    for(size_t i=0; i<N; i++)
    {
        if (!executeASTNode(statements[i]))
            return false;
    }

    // check that the operations stack is empty
    if (!m_opStack.empty())
    {
        error("SSACreator::process - Operand stack is not empty when it should be");
        // We've missed some operations :(
        return false;
    }
    return true;
}

bool SSACreator::addOperand(operand_t::type_t type, const varInfo &info, uint32_t &index)
{
    operand_t op;
    op.type = type;
    op.info = info;

    /* check if this name is already in the operand list */
    uint32_t dummy;
    if (findIdentifier(info.txt,dummy))
    {
        //note: do not produce error text here
        //better reporting can be done at higher
        //levels.
        return false;
    }

    m_operandList->push_back(op);
    index = m_operandList->size()-1;
    return true;
}

bool SSACreator::addIntegerOperand(operand_t::type_t type, const varInfo &info, uint32_t &index)
{
    operand_t op;
    op.type = type;
    op.info = info;
    op.info.txt.clear();

    // integer are unnamed: they can always be added onto the operand stack

    m_operandList->push_back(op);
    index = m_operandList->size()-1;
    return true;
}

uint32_t SSACreator::createIntermediate(int32_t intBits, int32_t fracBits)
{
    operand_t op;
    op.type = operand_t::TypeIntermediate;
    op.info.intBits = intBits;
    op.info.fracBits = fracBits;

    uint32_t index = m_operandList->size();

    std::ostringstream ss;
    ss << m_tempPrefix << index;
    op.info.txt = ss.str();

    m_operandList->push_back(op);
    return index;
}

bool SSACreator::findIdentifier(const std::string &name, uint32_t &index)
{
    size_t N = m_operandList->size();
    for(size_t i=0; i<N; i++)
    {
        if (m_operandList->at(i).info.txt == name)
        {
            index = i;
            return true;
        }
    }
    return false;
}

void SSACreator::determineWordlength(const operand_t &var, int32_t &intBits, int32_t &fracBits)
{
    switch(var.type)
    {
    case operand_t::TypeInput:
    case operand_t::TypeIntermediate:
    case operand_t::TypeOutput:
        intBits = var.info.intBits;
        fracBits = var.info.fracBits;
        break;
    case operand_t::TypeInteger:
        // calculate the number of bits needed to represent the integer
        intBits = static_cast<int32_t>(pow(2.0,ceil(log10((float)var.info.intVal)/log10(2.0))))+1;
        fracBits = 0;
        break;
    default:
    case operand_t::TypeCSD:
        // fixme:
        intBits = 0;
        fracBits = 0;
        break;
    }
}

bool SSACreator::executeASTNode(const ASTNodePtr node)
{
    if (node->left != 0)
    {
        executeASTNode(node->left);
    }

    if (node->right != 0)
        executeASTNode(node->right);

    uint32_t index;
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
        if (!addOperand(operand_t::TypeCSD, node->info, index))
        {
            error("CSD name already in use!");
            return false;
        }
        return true;
    case ASTNode::NodeInteger:
        // Literal integer, create an integer on the stack
        //
        addIntegerOperand(operand_t::TypeInteger, node->info, index);
        m_opStack.push_back(index);
        return true;
    case ASTNode::NodeInput:        
        // INPUT definition, no need to add to the stack
        // it's referenced by name
        if (!addOperand(operand_t::TypeInput, node->info, index))
        {
            error("Input name already in use!");
            return false;
        }
        return true;        
    case ASTNode::NodeIdent:
        // resolve the identifier by name
        if (!findIdentifier(node->info.txt, index))
        {
            error("Identifier not found");
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
            error("NodeAssign - not enough operands on the stack");
            return false;
        }

        // one item at top of stack:
        //
        uint32_t arg1_idx = m_opStack.back();
        const operand_t &arg1 = m_operandList->at(arg1_idx);
        m_opStack.pop_back();

        // create an output variable
        if (!addOperand(operand_t::TypeOutput, node->info, index))
        {
            error("Output name already in use!");
            return false;
        }

        operand_t *result = &(m_operandList->operator[](index));
        result->info.intBits = arg1.info.intBits;
        result->info.fracBits = arg1.info.fracBits;

        // create an SSA entry
        SSANode node;
        node.operation = SSANode::OP_Assign;
        node.var1 = arg1_idx;
        node.var2 = 0;
        node.var3 = index;
        m_ssaList->push_back(node);

        return true;
    }
    case ASTNode::NodeAdd:
    {
        // sanity checking
        if (m_opStack.size() < 2)
        {
            error("NodeAdd - not enough operands on the stack");
            // not enough operands!
            return false;
        }

        // two items at top of stack:
        //
        uint32_t arg1_idx = m_opStack.back();
        const operand_t &arg1 = m_operandList->at(arg1_idx);
        m_opStack.pop_back();
        uint32_t arg2_idx = m_opStack.back();
        const operand_t &arg2 = m_operandList->at(arg2_idx);
        m_opStack.pop_back();

        int32_t arg1_intBits;
        int32_t arg2_intBits;
        int32_t arg1_fracBits;
        int32_t arg2_fracBits;
        determineWordlength(arg1, arg1_intBits, arg1_fracBits);
        determineWordlength(arg2, arg2_intBits, arg2_fracBits);
        uint32_t index = createIntermediate(std::max(arg1_intBits, arg2_intBits)+1,
                                            std::max(arg1_fracBits,arg2_fracBits));
        m_opStack.push_back(index);

        // create an SSA entry
        SSANode node;
        node.operation = SSANode::OP_Add;
        node.var1 = arg1_idx;
        node.var2 = arg2_idx;
        node.var3 = index;
        m_ssaList->push_back(node);

        return true;
    }
    case ASTNode::NodeSub:
    {
        // sanity checking
        if (m_opStack.size() < 2)
        {
            error("NodeSub - not enough operands on the stack");
            // not enough operands!
            return false;
        }

        // two items at top of stack:
        //
        uint32_t arg1_idx = m_opStack.back();
        const operand_t &arg1 = m_operandList->at(arg1_idx);
        m_opStack.pop_back();
        uint32_t arg2_idx = m_opStack.back();
        const operand_t &arg2 = m_operandList->at(arg2_idx);
        m_opStack.pop_back();

        int32_t arg1_intBits;
        int32_t arg2_intBits;
        int32_t arg1_fracBits;
        int32_t arg2_fracBits;
        determineWordlength(arg1, arg1_intBits, arg1_fracBits);
        determineWordlength(arg2, arg2_intBits, arg2_fracBits);
        uint32_t index = createIntermediate(std::max(arg1_intBits, arg2_intBits)+1,
                                            std::max(arg1_fracBits,arg2_fracBits));
        m_opStack.push_back(index);

        // create an SSA entry
        SSANode node;
        node.operation = SSANode::OP_Sub;
        node.var1 = arg1_idx;
        node.var2 = arg2_idx;
        node.var3 = index;
        m_ssaList->push_back(node);

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
        uint32_t arg1_idx = m_opStack.back();
        const operand_t &arg1 = m_operandList->at(arg1_idx);
        m_opStack.pop_back();
        uint32_t arg2_idx = m_opStack.back();
        const operand_t &arg2 = m_operandList->at(arg2_idx);
        m_opStack.pop_back();

        int32_t arg1_intBits;
        int32_t arg2_intBits;
        int32_t arg1_fracBits;
        int32_t arg2_fracBits;
        determineWordlength(arg1, arg1_intBits, arg1_fracBits);
        determineWordlength(arg2, arg2_intBits, arg2_fracBits);
        uint32_t index = createIntermediate(arg1_intBits + arg2_intBits, arg1_fracBits + arg2_fracBits);
        m_opStack.push_back(index);

        // create an SSA entry
        SSANode node;
        node.operation = SSANode::OP_Mul;
        node.var1 = arg1_idx;
        node.var2 = arg2_idx;
        node.var3 = index;
        m_ssaList->push_back(node);

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

        // two items at top of stack:
        //
        uint32_t arg1_idx = m_opStack.back();
        const operand_t &arg1 = m_operandList->at(arg1_idx);
        m_opStack.pop_back();

        int32_t arg1_intBits;
        int32_t arg1_fracBits;
        determineWordlength(arg1, arg1_intBits, arg1_fracBits);
        uint32_t index = createIntermediate(arg1_intBits, arg1_fracBits);
        m_opStack.push_back(index);

        // create an SSA entry
        SSANode node;
        node.operation = SSANode::OP_Negate;
        node.var1 = arg1_idx;
        node.var2 = 0;
        node.var3 = index;
        m_ssaList->push_back(node);

        return true;
    }
    } // end switch
    return true;
}

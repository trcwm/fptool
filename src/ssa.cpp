/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Single static assignment intermediate representation
                for code generation.

  Author: Niels A. Moseley

*/

#include "ssa.h"
#include "csd.h"
#include <math.h>
#include <sstream>

namespace SSA
{

    operandIndex createNewTemporary(ssaOperands_t &operands, int32_t intbits, int32_t fracbits)
    {
        operand_t newop;
        newop.type = operand_t::TypeIntermediate;
        newop.info.intBits = intbits;
        newop.info.fracBits = fracbits;

        uint32_t index = operands.size();
        std::ostringstream ss;
        ss << "tmp" << index;
        newop.info.txt = ss.str();

        operands.push_back(newop);
        return index-1;
    }

    operandIndex createAddNode(ssaList_t &list, ssa_iterator where, ssaOperands_t &operands, operandIndex s1, operandIndex s2)
    {
        // create a new temporary result operand
        if ((s1 >= operands.size()) || (s2 >= operands.size()))
        {
            throw std::runtime_error("createAddNode: operands index out of bounds!");
        }

        int32_t intbits = std::max(operands[s1].info.intBits, operands[s2].info.intBits)+1;
        int32_t fracbits = std::max(operands[s1].info.fracBits, operands[s2].info.fracBits);
        operandIndex lhs_index = createNewTemporary(operands, intbits, fracbits);

        SSANode n;
        n.operation = SSANode::OP_Add;
        n.var1 = s1;
        n.var2 = s2;
        n.var3 = lhs_index;

        list.insert(where, n);
        return lhs_index;
    }

    operandIndex createSubNode(ssaList_t &list, ssa_iterator where, ssaOperands_t &operands, operandIndex s1, operandIndex s2)
    {
        // create a new temporary result operand
        if ((s1 >= operands.size()) || (s2 >= operands.size()))
        {
            throw std::runtime_error("createSubNode: operands index out of bounds!");
        }

        int32_t intbits = std::max(operands[s1].info.intBits, operands[s2].info.intBits)+1;
        int32_t fracbits = std::max(operands[s1].info.fracBits, operands[s2].info.fracBits);
        operandIndex lhs_index = createNewTemporary(operands, intbits, fracbits);

        SSANode n;
        n.operation = SSANode::OP_Sub;
        n.var1 = s1;
        n.var2 = s2;
        n.var3 = lhs_index;

        list.insert(where, n);
        return lhs_index;
    }

    operandIndex createMulNode(ssaList_t &list, ssa_iterator where, ssaOperands_t &operands, operandIndex s1, operandIndex s2)
    {
        // create a new temporary result operand
        if ((s1 >= operands.size()) || (s2 >= operands.size()))
        {
            throw std::runtime_error("createMulNode: operands index out of bounds!");
        }

        int32_t intbits = operands[s1].info.intBits + operands[s2].info.intBits;
        int32_t fracbits = operands[s1].info.fracBits + operands[s2].info.fracBits;
        operandIndex lhs_index = createNewTemporary(operands, intbits, fracbits);

        SSANode n;
        n.operation = SSANode::OP_Mul;
        n.var1 = s1;
        n.var2 = s2;
        n.var3 = lhs_index;

        list.insert(where, n);
        return lhs_index;
    }

    operandIndex createNegateNode(ssaList_t &list, ssa_iterator where, ssaOperands_t &operands, operandIndex s1)
    {
        // create a new temporary result operand
        if (s1 >= operands.size())
        {
            throw std::runtime_error("createNegateNode: operands index out of bounds!");
        }

        int32_t intbits = operands[s1].info.intBits;
        int32_t fracbits = operands[s1].info.fracBits;
        operandIndex lhs_index = createNewTemporary(operands, intbits, fracbits);

        SSANode n;
        n.operation = SSANode::OP_Negate;
        n.var1 = s1;
        n.var2 = 0;
        n.var3 = lhs_index;

        list.insert(where, n);
        return lhs_index;
    }

    void createAssignNode(ssaList_t &list, ssa_iterator where, ssaOperands_t &operands, operandIndex output, operandIndex s1)
    {
        // create a new temporary result operand
        if ((s1 >= operands.size()) || (output >= operands.size()))
        {
            throw std::runtime_error("createAssignNode: operands index out of bounds!");
        }

        //FIXME: check if bits are equal

        int32_t intbits = operands[s1].info.intBits;
        int32_t fracbits = operands[s1].info.fracBits;

        SSANode n;
        n.operation = SSANode::OP_Assign;
        n.var1 = s1;
        n.var2 = 0;
        n.var3 = output;

        list.insert(where, n);
    }

} // end namespace


bool SSACreator::process(const statements_t &statements, SSA::ssaList_t &ssaList, SSA::ssaOperands_t &ssaOperandList)
{
    m_lastError.clear();
    m_opStack.clear();
    m_ssaList = &ssaList;
    m_operands = &ssaOperandList;

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

bool SSACreator::addOperand(SSA::operand_t::type_t type, const varInfo &info, uint32_t &index)
{
    SSA::operand_t op;
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

    m_operands->push_back(op);
    index = m_operands->size()-1;
    return true;
}

bool SSACreator::addIntegerOperand(SSA::operand_t::type_t type, const varInfo &info, uint32_t &index)
{
    SSA::operand_t op;
    op.type = type;
    op.info = info;
    op.info.txt.clear();

    // integer are unnamed: they can always be added onto the operand stack

    m_operands->push_back(op);
    index = m_operands->size()-1;
    return true;
}

uint32_t SSACreator::createIntermediate(int32_t intBits, int32_t fracBits)
{
    SSA::operand_t op;
    op.type = SSA::operand_t::TypeIntermediate;
    op.info.intBits = intBits;
    op.info.fracBits = fracBits;

    uint32_t index = m_operands->size();

    std::ostringstream ss;
    ss << m_tempPrefix << index;
    op.info.txt = ss.str();

    m_operands->push_back(op);
    return index;
}

bool SSACreator::findIdentifier(const std::string &name, uint32_t &index)
{
    size_t N = m_operands->size();
    for(size_t i=0; i<N; i++)
    {
        if (m_operands->at(i).info.txt == name)
        {
            index = i;
            return true;
        }
    }
    return false;
}

void SSACreator::determineWordlength(const SSA::operand_t &var, int32_t &intBits, int32_t &fracBits)
{
    csd_t csd;
    switch(var.type)
    {
    case SSA::operand_t::TypeInput:
    case SSA::operand_t::TypeIntermediate:
    case SSA::operand_t::TypeOutput:
        intBits = var.info.intBits;
        fracBits = var.info.fracBits;
        break;
    case SSA::operand_t::TypeInteger:
        // calculate the number of bits needed to represent the integer
        intBits = static_cast<int32_t>(pow(2.0,ceil(log10((float)var.info.intVal)/log10(2.0))))+1;
        fracBits = 0;
        break;
    case SSA::operand_t::TypeCSD:
        if (convertToCSD(var.info.csdFloat, var.info.csdBits, csd))
        {
            intBits = csd.intBits;
            fracBits = csd.fracBits;
            std::cout << "CSD: Q(" << intBits << "," << fracBits << ") -> " << csd.value << "\n";
        }
        else
        {
            throw std::runtime_error("SSACreator: cannot determine word-length of CSD");
        }
        break;
    default:
        intBits = 0;
        fracBits = 0;
        throw std::runtime_error("SSACreator: undetermined operand type");
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
        if (!addOperand(SSA::operand_t::TypeCSD, node->info, index))
        {
            error("SSACreator - CSD name already in use!");
            return false;
        }
        return true;
    case ASTNode::NodeFloat:
        error("SSACreator - found an unsupported float literal!");
        return false;
    case ASTNode::NodeInteger:
        // Literal integer, create an integer on the stack
        //
        addIntegerOperand(SSA::operand_t::TypeInteger, node->info, index);
        m_opStack.push_back(index);
        return true;
    case ASTNode::NodeInput:        
        // INPUT definition, no need to add to the stack
        // it's referenced by name
        if (!addOperand(SSA::operand_t::TypeInput, node->info, index))
        {
            error("Input name already in use!");
            return false;
        }
        return true;
    case ASTNode::NodeIdent:
        // resolve the identifier by name
        if (!findIdentifier(node->info.txt, index))
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
        const SSA::operand_t &arg1 = m_operands->at(arg1_idx);
        m_opStack.pop_back();

        // create an output variable
        if (!addOperand(SSA::operand_t::TypeOutput, node->info, index))
        {
            error("Output name already in use!");
            return false;
        }

        SSA::operand_t *result = &(m_operands->operator[](index));
        result->info.intBits = arg1.info.intBits;
        result->info.fracBits = arg1.info.fracBits;

        // create an SSA entry
        SSA::SSANode node;
        node.operation = SSA::SSANode::OP_Assign;
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
            error("ASTNode::NodeAdd - not enough operands on the stack");
            // not enough operands!
            return false;
        }

        // two items at top of stack:
        //
        uint32_t arg1_idx = m_opStack.back();
        const SSA::operand_t &arg1 = m_operands->at(arg1_idx);
        m_opStack.pop_back();
        uint32_t arg2_idx = m_opStack.back();
        const SSA::operand_t &arg2 = m_operands->at(arg2_idx);
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
        SSA::SSANode node;
        node.operation = SSA::SSANode::OP_Add;
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
            error("ASTNode::NodeSub - not enough operands on the stack");
            // not enough operands!
            return false;
        }

        // two items at top of stack:
        //
        uint32_t arg1_idx = m_opStack.back();
        const SSA::operand_t &arg1 = m_operands->at(arg1_idx);
        m_opStack.pop_back();
        uint32_t arg2_idx = m_opStack.back();
        const SSA::operand_t &arg2 = m_operands->at(arg2_idx);
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
        SSA::SSANode node;
        node.operation = SSA::SSANode::OP_Sub;
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
        const SSA::operand_t &arg1 = m_operands->at(arg1_idx);
        m_opStack.pop_back();
        uint32_t arg2_idx = m_opStack.back();
        const SSA::operand_t &arg2 = m_operands->at(arg2_idx);
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
        SSA::SSANode node;
        node.operation = SSA::SSANode::OP_Mul;
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
        const SSA::operand_t &arg1 = m_operands->at(arg1_idx);
        m_opStack.pop_back();

        int32_t arg1_intBits;
        int32_t arg1_fracBits;
        determineWordlength(arg1, arg1_intBits, arg1_fracBits);
        uint32_t index = createIntermediate(arg1_intBits, arg1_fracBits);
        m_opStack.push_back(index);

        // create an SSA entry
        SSA::SSANode node;
        node.operation = SSA::SSANode::OP_Negate;
        node.var1 = arg1_idx;
        node.var2 = 0;
        node.var3 = index;
        m_ssaList->push_back(node);

        return true;
    }
    } // end switch
    return true;
}

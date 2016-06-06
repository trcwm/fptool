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

operandIndex SSAObject::createNewTemporary(int32_t intbits, int32_t fracbits)
{
    operand_t newop;
    newop.type = operand_t::TypeIntermediate;
    newop.info.intBits = intbits;
    newop.info.fracBits = fracbits;

    uint32_t index = m_operands.size();
    std::ostringstream ss;
    ss << "tmp" << index;
    newop.info.txt = ss.str();

    m_operands.push_back(newop);
    return index;
}

operandIndex SSAObject::createAddNode(ssa_iterator where, operandIndex s1, operandIndex s2)
{
    // create a new temporary result operand
    if ((s1 >= m_operands.size()) || (s2 >= m_operands.size()))
    {
        throw std::runtime_error("createAddNode: operands index out of bounds!");
    }

    int32_t intbits = std::max(m_operands[s1].info.intBits, m_operands[s2].info.intBits)+1;
    int32_t fracbits = std::max(m_operands[s1].info.fracBits, m_operands[s2].info.fracBits);
    operandIndex lhs_index = createNewTemporary(intbits, fracbits);

    SSANode n;
    n.operation = SSANode::OP_Add;
    n.var1 = s1;
    n.var2 = s2;
    n.var3 = lhs_index;

    m_list.insert(where, n);
    return lhs_index;
}

operandIndex SSAObject::createSubNode(ssa_iterator where, operandIndex s1, operandIndex s2)
{
    // create a new temporary result operand
    if ((s1 >= m_operands.size()) || (s2 >= m_operands.size()))
    {
        throw std::runtime_error("createSubNode: operands index out of bounds!");
    }

    int32_t intbits = std::max(m_operands[s1].info.intBits, m_operands[s2].info.intBits)+1;
    int32_t fracbits = std::max(m_operands[s1].info.fracBits, m_operands[s2].info.fracBits);
    operandIndex lhs_index = createNewTemporary(intbits, fracbits);

    SSANode n;
    n.operation = SSANode::OP_Sub;
    n.var1 = s1;
    n.var2 = s2;
    n.var3 = lhs_index;

    m_list.insert(where, n);
    return lhs_index;
}

operandIndex SSAObject::createMulNode(ssa_iterator where, operandIndex s1, operandIndex s2)
{
    // create a new temporary result operand
    if ((s1 >= m_operands.size()) || (s2 >= m_operands.size()))
    {
        throw std::runtime_error("createMulNode: operands index out of bounds!");
    }

    int32_t intbits = m_operands[s1].info.intBits + m_operands[s2].info.intBits;
    int32_t fracbits = m_operands[s1].info.fracBits + m_operands[s2].info.fracBits;
    operandIndex lhs_index = createNewTemporary(intbits, fracbits);

    SSANode n;
    n.operation = SSANode::OP_Mul;
    n.var1 = s1;
    n.var2 = s2;
    n.var3 = lhs_index;

    m_list.insert(where, n);
    return lhs_index;
}

operandIndex SSAObject::createNegateNode( ssa_iterator where, operandIndex s1)
{
    // create a new temporary result operand
    if (s1 >= m_operands.size())
    {
        throw std::runtime_error("createNegateNode: operands index out of bounds!");
    }

    int32_t intbits = m_operands[s1].info.intBits;
    int32_t fracbits = m_operands[s1].info.fracBits;
    operandIndex lhs_index = createNewTemporary(intbits, fracbits);

    SSANode n;
    n.operation = SSANode::OP_Negate;
    n.var1 = s1;
    n.var2 = 0;
    n.var3 = lhs_index;

    m_list.insert(where, n);
    return lhs_index;
}

void SSAObject::createAssignNode(ssa_iterator where, operandIndex output, operandIndex s1)
{
    // create a new temporary result operand
    if ((s1 >= m_operands.size()) || (output >= m_operands.size()))
    {
        throw std::runtime_error("createAssignNode: operands index out of bounds!");
    }

    //FIXME: check if bits are equal
    int32_t intbits = m_operands[s1].info.intBits;
    int32_t fracbits = m_operands[s1].info.fracBits;

    SSANode n;
    n.operation = SSANode::OP_Assign;
    n.var1 = s1;
    n.var2 = 0;
    n.var3 = output;

    m_list.insert(where, n);
}

operandIndex SSAObject::createExtendLSBNode(ssa_iterator where, operandIndex s1, int32_t bits)
{
    // create a new temporary result operand
    if (s1 >= m_operands.size())
    {
        throw std::runtime_error("createExtendLSBNode: operands index out of bounds!");
    }

    int32_t intbits = m_operands[s1].info.intBits;
    int32_t fracbits = m_operands[s1].info.fracBits;
    operandIndex lhs_index = createNewTemporary(intbits, fracbits+bits);

    SSANode n;
    n.operation = SSANode::OP_ExtendLSBs;
    n.bits = bits;
    n.var1 = s1;
    n.var2 = 0;
    n.var3 = lhs_index;

    m_list.insert(where, n);
    return lhs_index;
}

/** add an operand and check if one already exists with the same name */
operandIndex SSAObject::addOperand(operand_t::type_t type, const varInfo &info)
{
    operand_t op;
    op.type = type;
    op.info = info;


    /* check if this name is already in the operand list */
    if ((type != operand_t::TypeInteger) && (existsIdent(info.txt)))
    {
        throw std::runtime_error("SSAObject::addOperand - named operand already exists!");
    }

    m_operands.push_back(op);
    return m_operands.size()-1;
}


bool SSAObject::existsIdent(const std::string &name)
{
    operandIndex dummy;
    return getOperandIndexByName(name, dummy);
}

bool SSAObject::getOperandIndexByName(const std::string &name, operandIndex &index)
{
    size_t N = m_operands.size();
    for(size_t i=0; i<N; i++)
    {
        if (m_operands[i].info.txt == name)
        {
            index = i;
            return true;
        }
    }
    return false;
}

bool SSACreator::process(const statements_t &statements, SSAObject &ssa)
{
    m_lastError.clear();
    m_opStack.clear();

    size_t N = statements.size();
    for(size_t i=0; i<N; i++)
    {
        if (!executeASTNode(statements[i], ssa))
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


void SSACreator::determineWordlength(const operand_t &var, int32_t &intBits, int32_t &fracBits)
{
    csd_t csd;
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
    case operand_t::TypeCSD:
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

bool SSACreator::executeASTNode(const ASTNodePtr node, SSAObject &ssa)
{
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

        index = ssa.createNegateNode(ssa.end(), arg1_idx);
        m_opStack.push_back(index);

        return true;
    }
    } // end switch
    return true;
}

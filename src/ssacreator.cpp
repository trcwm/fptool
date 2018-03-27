/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Create a single-static assignment data structure
                from tha abstract syntax tree.

  Author: Niels A. Moseley

*/

#include <sstream>
#include <memory>

#include "ssacreator.h"

using namespace SSA;

Creator::Creator() : m_ssa(0)
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

bool Creator::process(const AST::Statements &statements, const IdentDB &symbols, SSA::Program &ssa)
{
    m_ssa = &ssa;
    m_identDB = &symbols;

    try
    {
        for(ASTNode *node : statements.m_statements)
        {
            if (node != NULL)
            {
                node->accept(this);
            }
        }
        return true;
    }
    catch(std::runtime_error &e)
    {
        std::cout << e.what() << "\n";
        return false;
    }
}

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

    SSA::SharedOpPtr result;
    bool lockPrecision = false;
    std::stringstream ss;   // for error handling..

    switch(m_identDB->getType(node->m_identName))
    {
    case IdentDB::info_t::T_NOTFOUND:
        ss << "Identifier "<< node->m_identName << " not found";
        error(ss.str());
        break;
    case IdentDB::info_t::T_REG:
        // registers have a user defined precision, so
        // copy that.
        lockPrecision = true;
        result = std::make_shared<RegOperand>();
        result->m_identName = node->m_identName;
        result->m_intBits   = m_identDB->m_identifiers.at(node->m_identName).m_intBits;
        result->m_fracBits  = m_identDB->m_identifiers.at(node->m_identName).m_fracBits;
        break;
    case IdentDB::info_t::T_OUTPUT:
        // the precision of the output operands are
        // determined by the expression
        // so we need to set it here..
        result = std::make_shared<OutputOperand>();
        result->m_identName = node->m_identName;
        result->m_intBits   = arg1->m_intBits;
        result->m_fracBits  = arg1->m_fracBits;
        break;
    case IdentDB::info_t::T_INPUT:
        // we cannot assign to inputs!
        ss << "Cannot assign to INPUT identifier " << node->m_identName;
        error(ss.str());
        break;
    case IdentDB::info_t::T_TMP:
        // temporaries should not exist yet..
        ss << "Cannot assign to TMP identifier " << node->m_identName << ". This is probably an internal error.";
        error(ss.str());
        break;
    case IdentDB::info_t::T_CSD:
        // we cannot assign to CSDs!
        ss << "Cannot assign to CSD identifier " << node->m_identName;
        error(ss.str());
        break;
    default:
        error("Internal error");
        break;
    };

    SSA::OpAssign *assign = new SSA::OpAssign(arg1, result, lockPrecision);
    m_ssa->addStatement(assign);
}

void Creator::visit(const AST::CSDDeclaration *node)
{
    // CSD should already exist.. check this
    std::stringstream ss;
    if (!m_identDB->hasIdentifier(node->m_identName))
    {
        ss << "CSD identifier "<< node->m_identName << " not found, but should have been.";
        error(ss.str());
    }

    // create a CSD constant
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
    // INPUT should already exist.. check this
    std::stringstream ss;
    if (!m_identDB->hasIdentifier(node->m_identName))
    {
        ss << "INPUT identifier "<< node->m_identName << " not found, but should have been.";
        error(ss.str());
    }

    // create an input variable
    SSA::SharedOpPtr result = std::make_shared<InputOperand>();
    result->m_intBits   = node->m_intBits;
    result->m_fracBits  = node->m_fracBits;
    result->m_identName = node->m_identName;
    m_ssa->addOperand(result);
}


void Creator::visit(const AST::RegDeclaration *node)
{
    // REG should already exist.. check this
    std::stringstream ss;
    if (!m_identDB->hasIdentifier(node->m_identName))
    {
        ss << "REG identifier "<< node->m_identName << " not found, but should have been.";
        error(ss.str());
    }

    // create a register variable
    SSA::SharedOpPtr result = std::make_shared<RegOperand>();
    result->m_intBits   = node->m_intBits;
    result->m_fracBits  = node->m_fracBits;
    result->m_identName = node->m_identName;
    m_ssa->addOperand(result);
}

void Creator::visit(const AST::IntegerConstant *node)
{
    (void)node;
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
    (void)node;
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


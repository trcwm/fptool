
#include "ssaevaluator.h"

using namespace SSA;

Evaluator::Evaluator(Program &ssa) : m_ssa(&ssa)
{
    setupValues();
}

Evaluator::~Evaluator()
{

}

void Evaluator::setupValues()
{
    for(auto operand : m_ssa->m_operands)
    {
        m_values[operand->m_identName] = fplib::SFix(operand->m_intBits, operand->m_fracBits);
    }
}

bool Evaluator::runProgram()
{
    for(auto statement : m_ssa->m_statements)
    {
        if (!statement->accept(this))
        {
            return false;
        }
    }
    return true;
}


bool Evaluator::visit(const OpAssign *node)
{
    fplib::SFix op = m_values[node->m_op->m_identName];
    m_values[node->m_lhs->m_identName] = op;
    return true;
}

bool Evaluator::visit(const OpMul *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op1->m_identName]*m_values[node->m_op2->m_identName];
    return true;
}

bool Evaluator::visit(const OpAdd *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op1->m_identName]+m_values[node->m_op2->m_identName];
    if (node->m_noExtension)
    {
        // remove the additional MSB that was created by the
        // fplib add operator
        m_values[node->m_lhs->m_identName] = m_values[node->m_lhs->m_identName].removeMSBs(1);
    }
    return true;
}

bool Evaluator::visit(const OpSub *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op1->m_identName]-m_values[node->m_op2->m_identName];
    if (node->m_noExtension)
    {
        // remove the additional MSB that was created by the
        // fplib add operator
        m_values[node->m_lhs->m_identName] = m_values[node->m_lhs->m_identName].removeMSBs(1);
    }
    return true;
}

bool Evaluator::visit(const OpNegate *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op->m_identName].negate();
    return true;
}

bool Evaluator::visit(const OpCSDMul *node)
{
    fplib::SFix result;
    fplib::SFix opVal = m_values[node->m_op->m_identName];
    int32_t intBits = node->m_op->m_intBits;
    int32_t fracBits = node->m_op->m_fracBits;
    for(auto digit : node->m_csd.digits)
    {
        if (digit.sign > 0)
        {
            result = result + opVal.reinterpret(intBits+digit.power, fracBits-digit.power);
        }
        else
        {
            result = result - opVal.reinterpret(intBits+digit.power, fracBits-digit.power);
        }
    }

    // chop off any extended bits that will have formed by using
    // regular adds and subs.
    if (node->m_lhs->m_intBits < result.intBits())
    {
        result = result.removeMSBs(result.intBits() - node->m_lhs->m_intBits);
    }

    m_values[node->m_lhs->m_identName] = result;

    //fplib::SFix v(node->m_lhs->m_intBits, node->m_lhs->m_fracBits);
    //m_values[node->m_lhs->m_identName].addPowerOfTwo(// -= m_values[node->m_op->m_identName];
    return true;
}

bool Evaluator::visit(const OpTruncate *node)
{
    fplib::SFix tmp = m_values[node->m_op->m_identName];
    if (tmp.intBits() > node->m_intBits)
    {
        tmp = tmp.removeMSBs(tmp.intBits() - node->m_intBits);
    }
    else if (tmp.intBits() < node->m_intBits)
    {
        tmp = tmp.extendMSBs(node->m_intBits - tmp.intBits());
    }
    if (tmp.fracBits() > node->m_fracBits)
    {
        tmp = tmp.removeLSBs(tmp.fracBits() - node->m_fracBits);
    }
    else if (tmp.fracBits() < node->m_fracBits)
    {
        tmp = tmp.extendLSBs(node->m_fracBits - tmp.fracBits());
    }
    m_values[node->m_lhs->m_identName] = tmp;
    return true;
}

bool Evaluator::visit(const OpReinterpret *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op->m_identName].reinterpret(
                node->m_intBits, node->m_fracBits);
    return true;
}

bool Evaluator::visit(const OpExtendLSBs *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op->m_identName].extendLSBs(
                node->m_bits);
    return true;
}

bool Evaluator::visit(const OpExtendMSBs *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op->m_identName].extendMSBs(
                node->m_bits);
    return true;
}

bool Evaluator::visit(const OpRemoveLSBs *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op->m_identName].removeLSBs(
                node->m_bits);
    return true;
}

bool Evaluator::visit(const OpRemoveMSBs *node)
{
    m_values[node->m_lhs->m_identName] = m_values[node->m_op->m_identName].removeMSBs(
                node->m_bits);
    return true;
}

bool Evaluator::visit(const OperationSingle *node)
{
    return false; // unsupported
}

bool Evaluator::visit(const OperationDual *node)
{
    return false; // unsupported
}

bool Evaluator::visit(const OpPatchBlock *node)
{
    return false; // unsupported
}

bool Evaluator::visit(const OpNull *node)
{
    return false; // unsupported
}

bool Evaluator::compareToRefEvaluator(const Evaluator &reference,
                                 std::stringstream &report)
{
    bool ok = true;

    // walk through all the operands in the reference
    for(auto refop : reference.m_ssa->m_operands)
    {
        const fplib::SFix *refval = reference.getValuePtrByName(refop->m_identName);
        if (refval == NULL)
        {
            throw std::runtime_error("Evaluator::compareToReferenceEvaluator cannot find reference value!");
        }
        // check if this evaluator actually has this variable
        auto opIter = m_values.find(refop->m_identName);
        if (opIter != m_values.end())
        {
            if ((*opIter).second != *refval)
            {
                report << "Mismatch " << refop->m_identName << "\n";
                report << "  ref Q(" << refval->intBits() << "," << refval->fracBits() << ")\n";
                report << "      Q(" << opIter->second.intBits() << "," << opIter->second.fracBits() << ")\n";
                report << "  ref " << refval->toHexString() << "\n";
                report << "      " << (*opIter).second.toHexString() << "\n";
                ok = false;
            }
            else
            {
                report << "Matched " << refop->m_identName << "\n";
            }
        }
        else
        {
            report << "Skipping " << refop->m_identName << "\n";
        }
    }
    return ok;
}

void Evaluator::initInputsFromRefEvaluator(const Evaluator &reference)
{
    // walk through all the input operands in the reference
    for(auto op : reference.m_ssa->m_operands)
    {
        const SSA::InputOperand* inOp = dynamic_cast<const SSA::InputOperand*>(op.get());
        if (inOp != NULL)
        {
            // operand is an input operand; we need to set
            // a value
            fplib::SFix *vptr = getValuePtrByName(op->m_identName);
            const fplib::SFix *vptr_ref = reference.getValuePtrByName(op->m_identName);
            if ((vptr != NULL) && (vptr_ref != NULL))
            {
                vptr->copyValueFrom(vptr_ref);
            }
            else
            {
                std::stringstream ss;
                ss << "Evaluator could not find input variable :" << op->m_identName;
                throw std::runtime_error(ss.str());
            }
        }
    }
}

void Evaluator::dumpInputValues(std::stringstream &report) const
{
    report << "Input values:\n";

    // walk through all the input operands
    for(auto op : m_ssa->m_operands)
    {
        const SSA::InputOperand* inOp = dynamic_cast<const SSA::InputOperand*>(op.get());
        if (inOp != NULL)
        {
            report << "  " << inOp->m_identName << " = " << m_values.at(inOp->m_identName).toHexString() << "\n";
        }
    }
}

void Evaluator::dumpAllValues(std::stringstream &report) const
{
    report << "Values:\n";

    // walk through all operands
    for(auto op : m_ssa->m_operands)
    {
        const SSA::InputOperand* inOp = dynamic_cast<const SSA::InputOperand*>(op.get());
        const SSA::IntermediateOperand* tmpOp = dynamic_cast<const SSA::IntermediateOperand*>(op.get());
        const SSA::OutputOperand* outOp = dynamic_cast<const SSA::OutputOperand*>(op.get());
        if (inOp != NULL)
        {
            report << "In   " << inOp->m_identName << " = " << m_values.at(inOp->m_identName).toHexString() << "\n";
        }
        else if (tmpOp != NULL)
        {
            report << "Tmp  " << tmpOp->m_identName << " = " << m_values.at(tmpOp->m_identName).toHexString() << "\n";
        }
        else if (outOp != NULL)
        {
            report << "Out  " << outOp->m_identName << " = " << m_values.at(outOp->m_identName).toHexString() << "\n";
        }
        else
        {
            report << "     " << outOp->m_identName << "\n";
        }
    }
}

#if 0

#include "logging.h"
#include "ssaevaluator.h"

// ****************************************************************************************************
//
//     SSAEvaluator
//
// ****************************************************************************************************

/** Evaluate a SSA intermediate language program */
bool SSAEvaluator::process(const SSAObject &ssa)
{
    auto iter = ssa.begin();
    while(iter != ssa.end())
    {
        operandIndex op1 = iter->op1Idx;
        operandIndex op2 = iter->op2Idx;
        operandIndex op3 = iter->lhsIdx;

        fplib::SFix tmp;
        switch(iter->operation)
        {
        default:
        case SSANode::OP_Undefined:
            doLog(LOG_ERROR,"SSAEvaluator::process encountered unknown node ID=%d\n", iter->operation);
            return false;
        case SSANode::OP_Assign:
            if (!varExists(op1))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }
            m_variables[op3] = m_variables[op1];
            break;
        case SSANode::OP_Negate:
            if (!varExists(op1))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }
            m_variables[op3] = m_variables[op1].negate();
            break;
        case SSANode::OP_Add:
            if (!varExists(op1))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }
            if (!varExists(op2))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }
            m_variables[op3] = m_variables[op1]+m_variables[op2];
            break;
        case SSANode::OP_Sub:
            if (!varExists(op1))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }
            if (!varExists(op2))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }
            m_variables[op3] = m_variables[op1]-m_variables[op2];
            break;
        case SSANode::OP_Mul:
            if (!varExists(op1))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }
            if (!varExists(op2))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }            
            m_variables[op3] = m_variables[op1]*m_variables[op2];
            break;
        case SSANode::OP_Truncate:
            if (!varExists(op1))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }

            if (m_variables[op1].fracBits() >= iter->fbits)
            {
                tmp = m_variables[op1].removeLSBs(m_variables[op1].fracBits() - iter->fbits);
            }
            else
            {
                tmp = m_variables[op1].extendLSBs(iter->fbits - m_variables[op1].fracBits());
            }
            if (tmp.intBits() >= iter->bits)
            {
                m_variables[op3] = tmp.removeMSBs(tmp.intBits() - iter->bits);
            }
            else
            {
                m_variables[op3] = tmp.extendMSBs(iter->bits - tmp.intBits());
            }
            break;
        case SSANode::OP_Reinterpret:
            if (!varExists(op1))
            {
                doLog(LOG_ERROR,"SSAEvaluator::process variable with index %d does not exist!\n", op1);
                return false;
            }
            m_variables[op3] = m_variables[op1].reinterpret(iter->bits, iter->fbits);
            break;
        }
        doDebug(op3, m_variables[op3]);
        iter++;
    }
    return true;
}

void SSAEvaluator::createCSDConstant(uint32_t index, const csd_t &csd)
{
    setInputVariable(index, convertCSDToSFix(csd));
}

void SSAEvaluator::setInputVariable(uint32_t index, const fplib::SFix &value)
{
    m_variables[index] = value;
}

bool SSAEvaluator::getValue(uint32_t index, fplib::SFix &outVal)
{
    if (m_variables.find(index) == m_variables.end())
    {
        return false;
    }
    else
    {
        outVal = m_variables[index];
        return true;
    }
}



bool fuzzer(const SSAObject &reference, const SSAObject &subject, const uint32_t tests)
{
    SSAEvaluator refEval;
    SSAEvaluator subEval;

    refEval.setDebug(true);
    subEval.setDebug(true);

    // ************************************************
    //   set CSDs to their correct value
    // ************************************************
    auto iter = reference.beginOperands();
    uint32_t opIndex = 0;
    while(iter != reference.endOperands())
    {
        if (iter->type == operand_t::TypeCSD)
        {
            doLog(LOG_INFO,"Setting CSD (var index=%d) %s to %f\n", opIndex, iter->info.txt.c_str()
                  ,iter->info.csdFloat);
            refEval.createCSDConstant(opIndex, iter->info.csd);
            subEval.createCSDConstant(opIndex, iter->info.csd);
        }
        iter++;
        opIndex++;
    }

    // ************************************************
    //   run the tests
    // ************************************************

    for(uint32_t i=0; i<tests; i++)
    {
        iter = reference.beginOperands();
        uint32_t opIndex = 0;
        while(iter != reference.endOperands())
        {
            if (iter->type == operand_t::TypeInput)
            {
                //doLog(LOG_INFO,"Setting input (var index=%d) %s to zero\n", opIndex, iter->info.txt.c_str());
                fplib::SFix value = fplib::SFix(iter->info.intBits, iter->info.fracBits);
                value.randomizeValue();
                refEval.setInputVariable(opIndex, value);
                subEval.setInputVariable(opIndex, value);
                if (true)
                {
                    doLog(LOG_DEBUG,"Input id %d := %s\n", opIndex, value.toHexString().c_str());
                }
            }
            iter++;
            opIndex++;
        }

        // evaluate SSAs
        if (!refEval.process(reference))
        {
             doLog(LOG_ERROR,"Reference evaluation failed\n");
             return false;
        }

        if (!subEval.process(subject))
        {
            doLog(LOG_ERROR,"Subject evaluation failed\n");
            return false;
        }

        // compare outputs
        iter = reference.beginOperands();
        opIndex = 0;
        while(iter != reference.endOperands())
        {
            if (iter->type == operand_t::TypeOutput)
            {
                fplib::SFix v1,v2;
                if (!refEval.getValue(opIndex, v1))
                {
                    return false;
                }
                if (!subEval.getValue(opIndex, v2))
                {
                    return false;
                }

                if (v1 != v2)
                {
                    doLog(LOG_ERROR,"Variable %s mismatch:\n  (ref)%s\n  (subject)%s\n",
                        iter->info.txt.c_str(),
                        v1.toHexString().c_str(),
                        v2.toHexString().c_str());
                    return false;
                }
            }
            iter++;
            opIndex++;
        }
    }
    return true;
}

void SSAEvaluator::setDebug(bool debug)
{
    m_debug = debug;
}

#endif


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
        operandIndex op3 = iter->op3Idx;

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

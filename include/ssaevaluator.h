/*

    A single static assignment (SSA) expression evaluator class
    meant for fuzzing/comparing different SSA programs.

    Niels A. Moseley 2017
    23-12-2017

*/


#ifndef ssaevaluator_h
#define ssaevaluator_h

#include <map>
#include <stdint.h>
#include "logging.h"
#include "fplib.h"
#include "ssa.h"
#include "csd.h"

/** Evaluate a SSA intermediate language program */
class SSAEvaluator
{
public:
    SSAEvaluator() : m_debug(false)
    {
    }

    /** create a CSD constant */
    void createCSDConstant(uint32_t index, const csd_t &csd);

    /** set the value of an input. */
    void setInputVariable(uint32_t index, const fplib::SFix &value);

    /** evaluate the SSA program */
    bool process(const SSAObject &ssa);

    /** get the value of a variable with index.
        returns false if a variable was not found. */
    bool getValue(uint32_t index, fplib::SFix &outVal);

    /** set debug on/off */
    void setDebug(bool debug);

protected:
    bool varExists(uint32_t index)
    {
        return (m_variables.find(index) != m_variables.end());
    }

    void doDebug(uint32_t index, const fplib::SFix &v)
    {
        if (m_debug)
        {
            doLog(LOG_DEBUG, "varID %d := %s\n", index, v.toHexString().c_str());
        }
    }

    bool m_debug;

    // map of variables, stored by (non-contiguous) IDs.
    // used for numerical value storage rather than
    // precision data / housekeeping.
    std::map<uint32_t, fplib::SFix> m_variables;
};

bool fuzzer(const SSAObject &reference, const SSAObject &subject, const uint32_t tests);

#endif

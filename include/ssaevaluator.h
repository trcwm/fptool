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

namespace SSA
{

class Evaluator : public OperationVisitorBase
{
public:
    Evaluator(Program &ssa);
    virtual ~Evaluator();

    bool runProgram();

    fplib::SFix* valuePtr(const std::string &name);

    virtual bool visit(const OpAssign *node) override;
    virtual bool visit(const OpMul *node) override;
    virtual bool visit(const OpAdd *node) override;
    virtual bool visit(const OpSub *node) override;
    virtual bool visit(const OpNegate *node) override;
    virtual bool visit(const OpCSDMul *node) override;
    virtual bool visit(const OpTruncate *node) override;
    virtual bool visit(const OpReinterpret *node) override;

    virtual bool visit(const OpExtendLSBs *node) override;
    virtual bool visit(const OpExtendMSBs *node) override;
    virtual bool visit(const OpRemoveLSBs *node) override;
    virtual bool visit(const OpRemoveMSBs *node) override;

    virtual bool visit(const OperationSingle *node) override;
    virtual bool visit(const OperationDual *node) override;
    virtual bool visit(const OpPatchBlock *node) override;
    virtual bool visit(const OpNull *node) override;

protected:
    /** fill the m_values container */
    void setupValues();

    Program *m_ssa;
    std::map<std::string, fplib::SFix> m_values;  ///< values of all operands (owns object)
};


} // namespace

#if 0

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
#endif

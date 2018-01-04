/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Single static assignment intermediate representation
                for code generation.

  Author: Niels A. Moseley

*/

#ifndef ssa_h
#define ssa_h

#include <list>
#include <string>
#include <memory>   // shared_ptr
#include <iostream>

#include "utils.h"
#include "csd.h"
#include "fplib.h"

/** single static assignment namespace */
namespace SSA
{

class OperationBase;            // forward declaration
class OperationVisitorBase;     // forward declaration

// *****************************************
// **********   OPERAND CLASSES   **********
// *****************************************

/** SSA base class for operands */
class OperandBase
{
public:
    virtual ~OperandBase() {}

    int32_t     m_intBits;
    int32_t     m_fracBits;
    std::string m_identName;

protected:
    OperandBase()
    {
    }
};


/** SSA operand that represents an input */
class InputOperand : public OperandBase
{
};


/** SSA operand that represents an output */
class OutputOperand : public OperandBase
{
};

static uint32_t gs_tempIdx = 0;

/** SSA operand that represents an intermediate variable */
class IntermediateOperand : public OperandBase
{
public:

    /** create a new named intermediate operand */
    static std::shared_ptr<IntermediateOperand> createNewIntermediate()
    {
        std::shared_ptr<IntermediateOperand> obj = std::make_shared<IntermediateOperand>();
        obj->m_identName = stringf("_TMP%d", gs_tempIdx++);
        return obj;
    }

};



/** SSA operand that represents an input */
class CSDOperand : public OperandBase
{
public:
    csd_t   m_csd;
};


typedef std::shared_ptr<OperandBase> SharedOpPtr;

// *****************************************
// **********  OPERATION CLASSES  **********
// *****************************************

/** SSA operation base class */
class OperationBase
{
public:
    virtual ~OperationBase() {}

    /** accept a visitor and call visitor->visit(this); */
    virtual bool accept(OperationVisitorBase *visitor) = 0;
};


/** Operation with two arguments */
class OperationDual : public OperationBase
{
public:
    OperationDual(SharedOpPtr op1, SharedOpPtr op2, SharedOpPtr result)
        : m_lhs(result), m_op1(op1), m_op2(op2)
    {
    }

    SharedOpPtr m_lhs;
    SharedOpPtr m_op1;
    SharedOpPtr m_op2;
};


/** Operation with one argument */
class OperationSingle : public OperationBase
{
public:
    OperationSingle(SharedOpPtr op, SharedOpPtr lhs) :
        m_op(op), m_lhs(lhs)
    {
    }

    SharedOpPtr m_lhs;
    SharedOpPtr m_op;
};


class OpAdd : public OperationDual
{
public:
    OpAdd(SharedOpPtr op1, SharedOpPtr op2, SharedOpPtr result)
        : OperationDual(op1,op2, result)
    {
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;
};


class OpSub : public OperationDual
{
public:
    OpSub(SharedOpPtr op1, SharedOpPtr op2, SharedOpPtr result)
        : OperationDual(op1,op2, result)
    {
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;
};


class OpMul : public OperationDual
{
public:
    OpMul(SharedOpPtr op1, SharedOpPtr op2, SharedOpPtr result)
        : OperationDual(op1,op2, result)
    {
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;
};


class OpNegate : public OperationSingle
{
public:
    OpNegate(SharedOpPtr op, SharedOpPtr result)
        : OperationSingle(op, result)
    {
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;
};

class OpTruncate : public OperationSingle
{
public:
    OpTruncate(SharedOpPtr op, SharedOpPtr result)
        : OperationSingle(op, result)
    {
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;
};

class OpAssign : public OperationSingle
{
public:
    OpAssign(SharedOpPtr op, SharedOpPtr output)
        : OperationSingle(op, output)
    {
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;
};

// *****************************************
// **********  SSA VISITOR CLASS  **********
// *****************************************

/** Operation visitor base class */
class OperationVisitorBase
{
public:
    virtual bool visit(const OpAssign *node) = 0;
    virtual bool visit(const OpMul *node) = 0;
    virtual bool visit(const OpAdd *node) = 0;
    virtual bool visit(const OpSub *node) = 0;
    virtual bool visit(const OpTruncate *node) = 0;
    virtual bool visit(const OperationSingle *node) = 0;
    virtual bool visit(const OperationDual *node) = 0;
};

// *****************************************
// **********  SSA PROGRAM CLASS  **********
// *****************************************

/** Collection of SSA statements */
class Program
{
public:
    /** convenience function to add a new statement to the list */
    void addStatement(OperationBase *statement)
    {
        m_statements.push_back(statement);
    }

    /** convenience function to add a named operand to the operand list */
    void addOperand(SharedOpPtr operand)
    {
        m_operands.push_back(operand);
    }

    std::list<OperationBase*> m_statements;
    std::list<SharedOpPtr>    m_operands;
};

} // namespace

#endif

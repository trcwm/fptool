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

    /** check if the operand is a CSD type */
    virtual bool isCSD() const
    {
        return false;
    }

protected:

    // hide constructor so nobody can make this base class
    OperandBase()
    {
    }
};

typedef std::shared_ptr<OperandBase> SharedOpPtr;

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
    virtual bool isCSD() const override
    {
        return true;
    }

    csd_t   m_csd;
};




// *****************************************
// **********  OPERATION CLASSES  **********
// *****************************************

/** SSA operation base class */
class OperationBase
{
public:
    virtual ~OperationBase() {}

    /** check if this is a patchblock operation/instruction */
    virtual bool isPatchBlock() const
    {
        return false;
    }

    /** accept a visitor and call visitor->visit(this); */
    virtual bool accept(OperationVisitorBase *visitor) = 0;

    /** replace operand op1 with op2 if op1 is present */
    virtual void replaceOperand(const SharedOpPtr op1, SharedOpPtr op2) = 0;
};


/** Operation with two arguments */
class OperationDual : public OperationBase
{
public:
    OperationDual(SharedOpPtr op1, SharedOpPtr op2, SharedOpPtr result)
        : m_lhs(result), m_op1(op1), m_op2(op2)
    {
    }

    /** replace operand op1 with op2 if op1 is present */
    virtual void replaceOperand(const SharedOpPtr op1, SharedOpPtr op2) override;

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

    /** replace operand op1 with operand op2, if present */
    virtual void replaceOperand(const SharedOpPtr op1, SharedOpPtr op2) override;

    SharedOpPtr m_lhs;
    SharedOpPtr m_op;
};


class OpAdd : public OperationDual
{
public:
    OpAdd(SharedOpPtr op1, SharedOpPtr op2, SharedOpPtr result)
        : OperationDual(op1,op2, result)
    {
        //FIXME: with range checking, we could avoid
        //       additional bits!
        result->m_intBits  = std::max(op1->m_intBits, op2->m_intBits)+1;
        result->m_fracBits = std::max(op1->m_fracBits, op2->m_fracBits);
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
        //FIXME: with range checking, we could avoid
        //       additional bits!
        result->m_intBits  = std::max(op1->m_intBits, op2->m_intBits)+1;
        result->m_fracBits = std::max(op1->m_fracBits, op2->m_fracBits);
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
        //FIXME: this is true for signed*signed multiplications
        //  however, CSDs are sign-magnitude and will have
        //  even fewer bits to output
        result->m_intBits  = op1->m_intBits + op2->m_intBits - 1;
        result->m_fracBits = op1->m_fracBits + op2->m_fracBits;
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
        // FIXME: this is not correct!
        // -MAX_VAL does not have an equivalent +value
        // due to two's complement asymmetry.
        result->m_intBits  = op->m_intBits;
        result->m_fracBits = op->m_fracBits;
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;
};


class OpTruncate : public OperationSingle
{
public:
    OpTruncate(SharedOpPtr op, SharedOpPtr result, int32_t intBits, int32_t fracBits)
        : OperationSingle(op, result),
          m_intBits(intBits),
          m_fracBits(fracBits)
    {
        result->m_intBits  = op->m_intBits;
        result->m_fracBits = op->m_fracBits;
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    int32_t m_intBits;      ///< number of integer bits to truncate to
    int32_t m_fracBits;     ///< number of fractional bits to truncate to
};


class OpAssign : public OperationSingle
{
public:
    OpAssign(SharedOpPtr op, SharedOpPtr output)
        : OperationSingle(op, output)
    {
        output->m_intBits  = op->m_intBits;
        output->m_fracBits = op->m_fracBits;
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

};


class OpReinterpret: public OperationSingle
{
public:
    OpReinterpret(SharedOpPtr op, SharedOpPtr output, int32_t intBits, int32_t fracBits)
        : OperationSingle(op, output),
          m_intBits(intBits),
          m_fracBits(fracBits)
    {
        output->m_intBits = intBits;
        output->m_intBits = fracBits;
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    int32_t m_intBits;  ///< reinterpret to this integer bits spec
    int32_t m_fracBits; ///< reinterpret to this fractional bits spec
};


class OpExtendLSBs : public OperationSingle
{
public:
    OpExtendLSBs(SharedOpPtr op, SharedOpPtr output, int32_t bits)
        : OperationSingle(op, output),
          m_bits(bits)
    {
        //FIXME:
        //  check that bits >= 0
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    int32_t m_bits;     ///< number of bits to extend
};



class OpRemoveLSBs : public OperationSingle
{
public:
    OpRemoveLSBs(SharedOpPtr op, SharedOpPtr output, int32_t bits)
        : OperationSingle(op, output),
          m_bits(bits)
    {
        //FIXME:
        //  check that bits >= 0
        output->m_intBits = op->m_intBits;
        output->m_fracBits = op->m_fracBits - bits;
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    int32_t m_bits;     ///< number of bits to remove
};


class OpExtendMSBs : public OperationSingle
{
public:
    OpExtendMSBs(SharedOpPtr op, SharedOpPtr output, int32_t bits)
        : OperationSingle(op, output),
          m_bits(bits)
    {
        //FIXME:
        //  check that bits >= 0
        output->m_intBits = op->m_intBits + bits;
        output->m_fracBits = op->m_fracBits;
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    int32_t m_bits;     ///< number of bits to extend
};


class OpRemoveMSBs : public OperationSingle
{
public:
    OpRemoveMSBs(SharedOpPtr op, SharedOpPtr output, int32_t bits)
        : OperationSingle(op, output),
          m_bits(bits)
    {
        //FIXME:
        //  check that bits >= 0
        output->m_intBits = op->m_intBits - bits;
        output->m_fracBits = op->m_fracBits;
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    int32_t m_bits;     ///< number of bits to remove
};

/** A special operation that holds a sequence of instructions
    to be inserted into the top-level operations list.
    This object is primarily there to aid patching
    SSA sequences in visitor patterns, where the
    iterator can only replace the current
    instruction being processed.

    A pointer to the replaced instruction can be given to
    the constuctor. When the patch has been applied, replacing
    the original instruction, the user can delete it, if desired.
*/
class OpPatchBlock : public OperationBase
{
public:
    OpPatchBlock(const OperationBase *replacedInstruction) :
        m_replacedInstruction(replacedInstruction)
    {}

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** check if this is a patchblock operation/instruction */
    virtual bool isPatchBlock() const override
    {
        return true;
    }

    /** replace operand op1 with operand op2, if present */
    virtual void replaceOperand(const SharedOpPtr op1, SharedOpPtr op2) override;

    std::list<OperationBase*> m_instructions;
    const OperationBase *m_replacedInstruction;
};


/** A special node that represents a no-operation,
    primarily meant to insert into the SSA program
    to replace exising instructions in transform
    passes */
class OpNull : public OperationBase
{
public:
    OpNull() {}

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** replace operand op1 with operand op2, if present */
    virtual void replaceOperand(const SharedOpPtr op1, SharedOpPtr op2) override {}

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
    virtual bool visit(const OpReinterpret *node) = 0;

    virtual bool visit(const OpExtendLSBs *node) = 0;
    virtual bool visit(const OpExtendMSBs *node) = 0;
    virtual bool visit(const OpRemoveLSBs *node) = 0;
    virtual bool visit(const OpRemoveMSBs *node) = 0;

    virtual bool visit(const OperationSingle *node) = 0;
    virtual bool visit(const OperationDual *node) = 0;
    virtual bool visit(const OpPatchBlock *node) = 0;
    virtual bool visit(const OpNull *node) = 0;

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

    /** merge the OpPatchBlock instructions
        into the main instruction sequence and remove
        any NULL operations. */
    void applyPatches();

    std::list<OperationBase*> m_statements;
    std::list<SharedOpPtr>    m_operands;
};

} // namespace

#endif

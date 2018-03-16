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

    bool        m_usedFlag;     ///< flag is used to tell whether this operand is used in a program.
    int32_t     m_intBits;      ///< number of integer bits of the variable/operand.
    int32_t     m_fracBits;     ///< number of fractional bits of the variable/operand.
    std::string m_identName;    ///< name of the variable/operand.

    /** check if the operand is a CSD type */
    virtual bool isCSD() const
    {
        return false;
    }

protected:

    // hide constructor so nobody can make this base class
    OperandBase()
        : m_usedFlag(false),
          m_intBits(0),
          m_fracBits(0),
          m_identName("UNUSED")
    {}
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

/** SSA operand that represents a register */
class RegOperand : public OperandBase
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
        obj->m_identName = stringf("TMP%d", gs_tempIdx++);
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
    virtual void replaceOperand(const SharedOpPtr &op1, SharedOpPtr op2) = 0;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const = 0;

    /** clone the object */
    virtual OperationBase* clone() const
    {
        throw std::runtime_error("OperationBase::clone() called on abstract base class!");
    }
};


/** Operation with two arguments */
class OperationDual : public OperationBase
{
public:
    OperationDual(const SharedOpPtr &op1, const SharedOpPtr &op2, const SharedOpPtr &result)
        : m_lhs(result), m_op1(op1), m_op2(op2)
    {
    }

    /** replace operand op1 with op2 if op1 is present */
    virtual void replaceOperand(const SharedOpPtr &op1, SharedOpPtr op2) override;

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        throw std::runtime_error("OperationDual::clone() called on abstract base class!");
    }

    SharedOpPtr m_lhs;
    SharedOpPtr m_op1;
    SharedOpPtr m_op2;
};


/** Operation with one argument */
class OperationSingle : public OperationBase
{
public:
    OperationSingle(const SharedOpPtr &op, const SharedOpPtr &lhs) :
        m_op(op), m_lhs(lhs)
    {
    }

    /** replace operand op1 with operand op2, if present */
    virtual void replaceOperand(const SharedOpPtr &op1, SharedOpPtr op2) override;

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        throw std::runtime_error("OperationSingle::clone() called on abstract base class!");
    }

    SharedOpPtr m_lhs;
    SharedOpPtr m_op;
};


class OpAdd : public OperationDual
{
public:
    /** create an addition operator result = op1 + op2.
        @param op1 first input operand.
        @param op2 second input operand.
        @param result output operand.
        @param noExtension when true, no extension bit is added to the result.
        */
    OpAdd(const SharedOpPtr &op1, const SharedOpPtr &op2,
          const SharedOpPtr &result, bool noExtension = false)
        : OperationDual(op1,op2, result), m_noExtension(noExtension)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        m_lhs->m_intBits  = std::max(m_op1->m_intBits, m_op2->m_intBits);
        if (!m_noExtension)
        {
            m_lhs->m_intBits++;
        }
        m_lhs->m_fracBits = std::max(m_op1->m_fracBits, m_op2->m_fracBits);
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpAdd(*this);
    }

    bool m_noExtension;
};


class OpSub : public OperationDual
{
public:
    /** create an subtraction operator result = op1 - op2.
    @param op1 first input operand.
    @param op2 second input operand.
    @param result output operand.
    @param noExtension when true, no extension bit is added to the result.
    */
    OpSub(const SharedOpPtr &op1, const SharedOpPtr &op2,
          const SharedOpPtr &result, bool noExtension = false)
        : OperationDual(op1,op2, result), m_noExtension(noExtension)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        m_lhs->m_intBits  = std::max(m_op1->m_intBits, m_op2->m_intBits);
        if (!m_noExtension)
        {
            m_lhs->m_intBits++;
        }
        m_lhs->m_fracBits = std::max(m_op1->m_fracBits, m_op2->m_fracBits);
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpSub(*this);
    }

    bool m_noExtension;
};


class OpMul : public OperationDual
{
public:
    OpMul(const SharedOpPtr &op1, const SharedOpPtr &op2,
          const SharedOpPtr &result)
        : OperationDual(op1,op2, result)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        m_lhs->m_intBits  = m_op1->m_intBits + m_op2->m_intBits - 1;
        m_lhs->m_fracBits = m_op1->m_fracBits + m_op2->m_fracBits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpMul(*this);
    }
};


class OpCSDMul : public OperationSingle
{
public:
    OpCSDMul(const SharedOpPtr &op, const csd_t &csd, const std::string &csdName,
             const SharedOpPtr &result)
        : OperationSingle(op, result), m_csd(csd), m_csdName(csdName)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        //FIXME: this is true for signed*signed multiplications
        //  however, CSDs are sign-magnitude and will have
        //  even fewer bits to output
        //
        //  example (2^1 + 2^-3) * x
        //  where x is Q(n,m)
        //
        //  2^1  * x -> Q(n+1,m-1)
        //  2^-3 * x -> Q(n-3,m+3)
        //  after addition:
        //  Q(n+1,m-1) + Q(n-3,m+3) -> Q(n+2,m+3)
        //
        //  however: (2^1 - 2^-3) * x
        //  where x is Q(n,m)
        //
        //  2^1  * x -> Q(n+1,m-1)
        //  2^-3 * x -> Q(n-3,m+3)
        //  after addition:
        //  Q(n,m) - Q(n-3,m+3) -> Q(n,m+3)
        //  because (2^0 - 2^-3) < 2^0
        //
        //  if the first digit is positive and
        //  the second is negative or non-existent,
        //  the CSD coefficient is smaller or equal
        //  to the first digit and we don't need
        //  an additional expansion MSB.
        //

        int32_t Pmax = m_csd.digits.front().power;
        int32_t Pmin = m_csd.digits.back().power;

        m_lhs->m_intBits = Pmax + m_op->m_intBits;
        if ((m_csd.digits.size() > 1) && (m_csd.digits[0].sign != m_csd.digits[1].sign))
        {
            m_lhs->m_intBits++;
        }

        m_lhs->m_fracBits = -Pmin + m_op->m_fracBits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpCSDMul(*this);
    }

    csd_t m_csd;            ///< multiplication factor / constant
    std::string m_csdName;  ///< name of CSD factor / constant
};


class OpNegate : public OperationSingle
{
public:
    OpNegate(const SharedOpPtr &op, const SharedOpPtr &result)
        : OperationSingle(op, result)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        // FIXME: this is not correct!
        // -MAX_VAL does not have an equivalent +value
        // due to two's complement asymmetry.
        m_lhs->m_intBits  = m_op->m_intBits;
        m_lhs->m_fracBits = m_op->m_fracBits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpNegate(*this);
    }
};


class OpTruncate : public OperationSingle
{
public:
    OpTruncate(const SharedOpPtr &op, const SharedOpPtr &result,
               int32_t intBits, int32_t fracBits)
        : OperationSingle(op, result),
          m_intBits(intBits),
          m_fracBits(fracBits)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        m_lhs->m_intBits  = m_intBits;
        m_lhs->m_fracBits = m_fracBits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpTruncate(*this);
    }

    int32_t m_intBits;      ///< number of integer bits to truncate to
    int32_t m_fracBits;     ///< number of fractional bits to truncate to
};


class OpAssign : public OperationSingle
{
public:
    OpAssign(const SharedOpPtr &op, const SharedOpPtr &output, bool updatePrecision = true)
        : OperationSingle(op, output)
    {
        if (updatePrecision)
        {
            updateOutputPrecision();
        }
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        m_lhs->m_intBits  = m_op->m_intBits;
        m_lhs->m_fracBits = m_op->m_fracBits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpAssign(*this);
    }

};


class OpReinterpret: public OperationSingle
{
public:
    OpReinterpret(const SharedOpPtr &op, const SharedOpPtr &output,
                  int32_t intBits, int32_t fracBits)
        : OperationSingle(op, output),
          m_intBits(intBits),
          m_fracBits(fracBits)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        m_lhs->m_intBits = m_intBits;
        m_lhs->m_fracBits = m_fracBits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpReinterpret(*this);
    }

    int32_t m_intBits;  ///< reinterpret to this integer bits spec
    int32_t m_fracBits; ///< reinterpret to this fractional bits spec
};


class OpExtendLSBs : public OperationSingle
{
public:
    OpExtendLSBs(const SharedOpPtr &op, const SharedOpPtr &output, int32_t bits)
        : OperationSingle(op, output),
          m_bits(bits)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        //FIXME:
        //  check that bits >= 0
        m_lhs->m_intBits  = m_op->m_intBits;
        m_lhs->m_fracBits = m_op->m_fracBits + m_bits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpExtendLSBs(*this);
    }

    int32_t m_bits;     ///< number of bits to extend
};



class OpRemoveLSBs : public OperationSingle
{
public:
    OpRemoveLSBs(const SharedOpPtr &op, const SharedOpPtr &output, int32_t bits)
        : OperationSingle(op, output),
          m_bits(bits)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        //FIXME:
        //  check that bits >= 0
        m_lhs->m_intBits  = m_op->m_intBits;
        m_lhs->m_fracBits = m_op->m_fracBits - m_bits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpRemoveLSBs(*this);
    }

    int32_t m_bits;     ///< number of bits to remove
};


class OpExtendMSBs : public OperationSingle
{
public:
    OpExtendMSBs(const SharedOpPtr &op, const SharedOpPtr &output, int32_t bits)
        : OperationSingle(op, output),
          m_bits(bits)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        //FIXME:
        //  check that bits >= 0
        m_lhs->m_intBits  = m_op->m_intBits + m_bits;
        m_lhs->m_fracBits = m_op->m_fracBits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpExtendMSBs(*this);
    }

    int32_t m_bits;     ///< number of bits to extend
};


class OpRemoveMSBs : public OperationSingle
{
public:
    OpRemoveMSBs(const SharedOpPtr &op, const SharedOpPtr &output, int32_t bits)
        : OperationSingle(op, output),
          m_bits(bits)
    {
        updateOutputPrecision();
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        //FIXME:
        //  check that bits >= 0
        m_lhs->m_intBits  = m_op->m_intBits - m_bits;
        m_lhs->m_fracBits = m_op->m_fracBits;
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpRemoveMSBs(*this);
    }

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
    explicit OpPatchBlock(const OperationBase *replacedInstruction) :
        m_replacedInstruction(replacedInstruction)
    {
        //Note: we cannot call updateOutputPrecision here
        //  as there are no instructions to process yet..
    }

    /** accept a visitor */
    virtual bool accept(OperationVisitorBase *visitor) override;

    /** check if this is a patchblock operation/instruction */
    virtual bool isPatchBlock() const override
    {
        return true;
    }

    /** replace operand op1 with operand op2, if present */
    virtual void replaceOperand(const SharedOpPtr &op1, SharedOpPtr op2) override;

    /** add statement to the instruction list */
    void addStatement(OperationBase *statement)
    {
        m_statements.push_back(statement);
    }

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override
    {
        for(auto statement : m_statements)
        {
            statement->updateOutputPrecision();
        }
    }

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        throw std::runtime_error("OpPatchBlock::clone() is not suppoted!");
    }

    std::list<OperationBase*> m_statements;
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
    virtual void replaceOperand(const SharedOpPtr &op1, SharedOpPtr op2) override {}

    /** calculate and set the Q(n,m) precision of the
        LHS / output operand */
    virtual void updateOutputPrecision() const override {}

    /** clone the object */
    virtual OperationBase* clone() const override
    {
        return new OpNull(*this);
    }

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
    virtual bool visit(const OpNegate *node) = 0;
    virtual bool visit(const OpCSDMul *node) = 0;
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
    Program() {}

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

    /** calculate and set the Q(n,m) precision of the
        operands / variables */
    void updateOutputPrecisions()
    {
        for(auto statement : m_statements)
        {
            statement->updateOutputPrecision();
        }
    }

    /** copy constructor to safely duplicate all the statements */
    Program(const Program &obj)
    {
        m_operands = obj.m_operands;
        for (auto statement : obj.m_statements)
        {
            m_statements.push_back(statement->clone());
        }
    }

    std::list<OperationBase*> m_statements;
    std::list<SharedOpPtr>    m_operands;
};

} // namespace

#endif

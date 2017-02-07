/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Single static assignment intermediate representation
                for code generation.

  Author: Niels A. Moseley

*/

#ifndef ssa_h
#define ssa_h

#include <string>
#include <stdint.h>
#include <list>
#include <iostream>
#include "parser.h"


typedef size_t operandIndex;

/** Operand type for SSA */
struct operand_t
{
    enum type_t
    {
        TypeInteger,        // integer constant
        TypeCSD,            // canonical signed digit type
        TypeInput,          // pre-defined input variable
        TypeIntermediate,   // intermediate result var
        TypeOutput,         // output variable (cannot be read)
        TypeRemoved         // type has been removed
    };

    type_t  type;
    varInfo info;           // variable information (copied from Parser)
};


/** Single static assignment node

    Note: nodes can be synthesizable or non-synthesizable.
    For instance, when we have and Add node:

    var3 = var2 + var1

    ,where var1 and var2 have different Q() sizes,
    the operation is not synthesizable in that
    a VHDL or Verilog synthesis tool will not compile
    or produce erroneous result if the fractional
    parts of var1 and var2 are not equalized in length.
    Furthermore, to avoid overflow, an additional bit
    is needed at the MSB end.

    TODO: make a function to returns 'true' if an SSA node is
    synthesizable.

*/
struct SSANode
{
    enum operation_t
    {
        OP_Undefined,   // Node is undefined
        OP_Add,         // Add two variables var3 = var1 + var2
        OP_Sub,         // Subtract two variables var3 = var1 - var2
        OP_Mul,         // Multiply two variables var3 = var1 * var2
        OP_Div,         // Divide two variables var3 = var1 / var2
        OP_Negate,      // Sign reverse
        OP_Assign,      // Assign to an output/register variable
        OP_Reinterpret, // Reinterpret Q(n1,m2) to Q(n2,m2) where n1-m2 = n2-m2
        OP_Saturate,    // Saturate var3 = Saturate(var1, bits, fbits)
        OP_RemoveLSBs,  // Remove bits from the LSB side
        OP_ExtendLSBs,  // Add bits at the LSB side (zero bits)
        OP_RemoveMSBs,  // Remove bits at the MSB side
        OP_ExtendMSBs   // Add bits at the MSB side (sign extend)
    };

    /** constructor to initialize things to safe defaults */
    SSANode() : operation(OP_Undefined), var1(0),var2(0),var3(0),bits(0),fbits(0)
    {
    }

    operation_t operation;
    size_t      var1;   // Index of operand 1
    size_t      var2;   // Index of operand 2
    size_t      var3;   // Index of operand 3 (lhs)
    int32_t     bits;   // remove/extend/saturate (integer) bits
    int32_t     fbits;  // saturate (fractional) bits
};

typedef std::list<SSANode> ssaList_t;
typedef std::vector<operand_t> ssaOperands_t;
typedef std::list<SSANode>::iterator ssa_iterator;


/** SSA object holds variables and a list of SSA node describing operations */
class SSAObject
{
public:
    SSAObject() {}
    virtual ~SSAObject() {}

    /** create an Add node tmp = s1 + s2, returning the index
        of the new temp variable.
        s1 and s2 must be either TypeInput or TypeIntermediate.
    */
    operandIndex createAddNode(ssa_iterator where, operandIndex s1, operandIndex s2);

    /** create an Sub node tmp = s1 - s2, returning the index
        of the new temp variable.
        s1 and s2 must be either TypeInput or TypeIntermediate.
    */
    operandIndex createSubNode(ssa_iterator where, operandIndex s1, operandIndex s2);

    /** create a Mul node tmp = s1*s2, returning the index
         of the new temp variable.
         s1 and s2 must either be TypeInput, TypeIntermediate or TypeCSD.
    */
    operandIndex createMulNode(ssa_iterator where, operandIndex s1, operandIndex s2);

    /** create a Div node tmp = s1/s2, returning the index
         of the new temp variable.
         s1 and s2 must either be TypeInput, TypeIntermediate.
    */
    operandIndex createDivNode(ssa_iterator where, operandIndex s1, operandIndex s2);

    /** create a negate node, returning the index
        of the new temp variable.
        s1 and s2 must either be TypeInput, TypeIntermediate.
    */
    operandIndex createNegateNode(ssa_iterator where, operandIndex s1);

    /** create an assignment node.
        output must be TypeOutput, s1 must either be TypeInput, TypeIntermediate.
    */
    void createAssignNode(ssa_iterator where, operandIndex output, operandIndex s1);

    /** create an extendLSB node.
        s1 must either be TypeInput, TypeIntermediate.
    */
    operandIndex createExtendLSBNode(ssa_iterator where, operandIndex s1, int32_t bits);

    /** create an extendMSB node.
        s1 must either be TypeInput, TypeIntermediate.
    */
    operandIndex createExtendMSBNode(ssa_iterator where, operandIndex s1, int32_t bits);

    /** create a reinterpret node.
        s1 must either be TypeInput, TypeIntermediate.
    */
    operandIndex createReinterpretNode(ssa_iterator where, operandIndex s1, int32_t intbits, int32_t fracbits);


    /** create a new temporary operand and return its index */
    operandIndex createNewTemporary(int32_t intbits, int32_t fracbits);

    /** add an operand and check if one already exists with the same name */
    operandIndex addOperand(operand_t::type_t type, const varInfo &info);

    /** get an operand index by its name. returns true if found, false otherwise. */
    bool getOperandIndexByName(const std::string &name, operandIndex &index);

    /** get a copy of an operand at a certain index */
    operand_t getOperand(operandIndex index) const
    {
        if (index >= m_operands.size())
        {
            throw std::runtime_error("SSAObject::getOperands index out of bounds");
        }
        return m_operands[index];
    }

    /** mark an operand as removed */
    void markRemoved(operandIndex index)
    {
        if (index >= m_operands.size())
        {
            throw std::runtime_error("SSAObject::markRemoved index out of bounds");
        }
        m_operands[index].type = operand_t::TypeRemoved;
    }

    /** remove a node, return an iterator to the next item */
    ssa_iterator removeNode(ssa_iterator where)
    {
        return m_list.erase(where);
    }

    /** return an iterator pointing to the beginning of the operation list */
    ssa_iterator begin()
    {
        return m_list.begin();
    }

    /** return an iterator pointing to the end of the operation list */
    ssa_iterator end()
    {
        return m_list.end();
    }

    /** return an iterator pointing to the beginning of the operands list */
    std::vector<operand_t>::iterator beginOperands()
    {
        return m_operands.begin();
    }

    /** return an iterator pointing to the end of the operands list */
    std::vector<operand_t>::iterator endOperands()
    {
        return m_operands.end();
    }

    std::vector<operand_t>::const_iterator beginOperands() const
    {
        return m_operands.begin();
    }

    std::vector<operand_t>::const_iterator endOperands() const
    {
        return m_operands.end();
    }

    /** dump statements in human readable form */
    void dumpStatements(std::ostream &stream);

protected:

    /** check for the existance of an identifier */
    bool existsIdent(const std::string &name);

    ssaList_t       m_list;
    ssaOperands_t   m_operands;
};





/** Create a SSA intermediate language representation from
    Abstract Syntax Tree */
class SSACreator
{
public:
    SSACreator() {}

    /** Process a list of AST statements and produce a list of SSA statements,
        and a list of variables/operands.
        Returns false if an error occurred.
    */
    bool process(const statements_t &statements, SSAObject &ssa);

    /** Get a human readable version of the error */
    std::string getLastError() const
    {
        return m_lastError;
    }

protected:
    bool executeASTNode(ASTNode *node, SSAObject &ssa);

    /** determine the Q(n,m) wordlength of a variable */
    void determineWordlength(const operand_t &var, int32_t &intBits, int32_t &fracBits);

    /** emit an error in human readable form */
    void error(const std::string &errorstr)
    {
        std::cout << errorstr << std::endl;
        m_lastError = errorstr;
    }

    const statements_t  *m_statements;

    std::string         m_lastError;
    std::vector<uint32_t> m_opStack;
};

#endif

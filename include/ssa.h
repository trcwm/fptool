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
#include <iostream>
#include "parser.h"


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


/** Single static assignment node */
struct SSANode
{
    enum operation_t
    {
        OP_Add,         // Add two variables var3 = var1 + var2
        OP_Sub,         // Subtract two variables var3 = var1 - var2
        OP_Mul,         // Multiply two variables var3 = var1 * var2
        OP_Negate,      // Sign reverse
        OP_Assign,      // Assign to an output/register variable
        OP_Saturate,    // Saturate var3 = Saturate(var1, bits, fbits)
        OP_RemoveLSBs,  // Remove bits from the LSB side
        OP_ExtendLSBs,  // Add bits at the LSB side (zero bits)
        OP_RemoveMSBs,  // Remove bits at the MSB side
        OP_ExtendMSBs   // Add bits at the MSB side (sign extend)
    };

    operation_t operation;
    size_t      var1;   // Index of operand 1
    size_t      var2;   // Index of operand 2
    size_t      var3;   // Index of operand 3 (lhs)
    int32_t     bits;   // remove/extend/saturate (integer) bits
    int32_t     fbits;  // saturate (fractional) bits
};

typedef std::vector<SSANode> ssaList_t;
typedef std::vector<operand_t> ssaOperandList_t;


/** Create a SSA intermediate language representation from
    Abstract Syntax Tree */
class SSACreator
{
public:
    SSACreator() : m_tempPrefix("tmp") {}

    /** Process a list of AST statements and produce a list of SSA statements,
        and a list of variables/operands.
        Returns false if an error occurred.
    */
    bool process(const statements_t &statements, ssaList_t &ssaList, ssaOperandList_t &ssaOperandList);

    /** Get a human readable version of the error */
    std::string getLastError() const
    {
        return m_lastError;
    }

protected:
    bool executeASTNode(const ASTNodePtr node);

    /** Add a variable or CSD from AST
        and return its resulting index into
        the operand list.
    */
    bool addOperand(operand_t::type_t type, const varInfo &var, uint32_t &index);

    /** Add an integer operand to the operand stack */
    bool addIntegerOperand(operand_t::type_t type, const varInfo &var, uint32_t &index);

    /** create a intermediate operand/value of Q(intBits,fracBits) format
        and return the index into the operand list */
    uint32_t createIntermediate(int32_t intBits, int32_t fracBits);


    /** find an identifier by name and return its index,
        if found. */
    bool findIdentifier(const std::string &name, uint32_t &index);

    /** determine the Q(n,m) wordlength of a variable */
    void determineWordlength(const operand_t &var, int32_t &intBits, int32_t &fracBits);

    /** emit an error in human readable form */
    void error(const std::string &errorstr)
    {
        std::cout << errorstr << std::endl;
        m_lastError = errorstr;
    }

    const statements_t *m_statements;
    ssaList_t          *m_ssaList;
    ssaOperandList_t   *m_operandList;

    std::string         m_lastError;
    std::string         m_tempPrefix;
    std::vector<uint32_t> m_opStack;
};

#endif

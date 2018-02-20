/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  VHDL code generator

*/

#include "logging.h"
#include <algorithm>
#include "vhdlcodegen.h"

using namespace SSA;

VHDLCodeGen::VHDLCodeGen(std::ostream &os, Program &ssa) :
    m_os(os), m_ssa(&ssa), m_indent(0)
{

}

bool VHDLCodeGen::execute()
{
    doLog(LOG_INFO, "-----------------------\n");
    doLog(LOG_INFO, "  Running VHDLCodeGen\n");
    doLog(LOG_INFO, "-----------------------\n");

    m_indent = 2;

    m_os << m_prolog;

    genProcessHeader(m_indent);

    m_indent += 2;
    for(auto statement : m_ssa->m_statements)
    {
        if (!statement->accept(this))
            return false;
    }
    m_indent-=2;
    genIndent(m_indent);
    m_os << "end process;\n";

    m_os << m_epilog;
    return true;
}

void VHDLCodeGen::genIndent(uint32_t indent)
{
    for(uint32_t i=0; i<indent; i++)
    {
        m_os << " ";
    }
}

void VHDLCodeGen::genProcessHeader(uint32_t indent)
{
    //
    // <input signal documentation>
    // <output signal documentation>
    // proc_comb: process( <sensitivity list )
    //   <variable list>
    // begin
    //

    // generate documentation for output signals
    m_os << "  -- *** OUTPUT SIGNALS ***\n";

    for(auto operand : m_ssa->m_operands)
    {
        OutputOperand *op = dynamic_cast<OutputOperand*>(operand.get());
        if (op != NULL)
        {
            genIndent(m_indent);
            m_os << "-- signal " << op->m_identName.c_str();
            m_os << " : SIGNED(" << op->m_intBits + op->m_fracBits-1 << " downto 0);  --";
            m_os << " Q(" << op->m_intBits << "," << op->m_fracBits << ");\n";
        }
    }

    // generate documentation for input signals
    m_os << "\n";
    m_os << "  -- *** INPUT SIGNALS ***\n";
    for(auto operand : m_ssa->m_operands)
    {
        InputOperand *op = dynamic_cast<InputOperand*>(operand.get());
        if (op != NULL)
        {
            genIndent(m_indent);
            m_os << "-- signal " << op->m_identName.c_str();
            m_os << " : SIGNED(" << op->m_intBits + op->m_fracBits-1 << " downto 0);  --";
            m_os << " Q(" << op->m_intBits << "," << op->m_fracBits << ");\n";
        }
    }

    // generate process header with sensitivity list
    m_os << "\n";
    m_os << "  -------------------\n";
    m_os << "  -- PROCESS BLOCK --\n";
    m_os << "  -------------------\n";

    genIndent(indent);
    m_os << "proc_comb: process(";

    bool isFirst = true;
    for(auto operand : m_ssa->m_operands)
    {
        InputOperand *op = dynamic_cast<InputOperand*>(operand.get());
        if (op != NULL)
        {
            if (!isFirst)
                m_os << ",";
            m_os << op->m_identName.c_str();
            isFirst = false;
        }
    }
    m_os << ")\n"; // terminate process header
    m_indent+=2;

    // write the variable list
    for(auto operand : m_ssa->m_operands)
    {
        IntermediateOperand *op = dynamic_cast<IntermediateOperand*>(operand.get());
        if (op != NULL)
        {
            genIndent(m_indent);
            m_os << "variable " << op->m_identName.c_str();
            m_os << " : SIGNED(" << op->m_intBits + op->m_fracBits-1 << " downto 0);  --";
            m_os << " Q(" << op->m_intBits << "," << op->m_fracBits << ");\n";

            //doLog(LOG_INFO, "Creating variable %s\n", op->m_identName.c_str());
        }
        else
        {
            //doLog(LOG_INFO, "Skipping variable %s\n", operand->m_identName.c_str());
        }
    }
    m_indent-=2;
    genIndent(m_indent);
    m_os << "begin\n";
}


bool VHDLCodeGen::visit(const OpAssign *node)
{
    genIndent(m_indent);
    OutputOperand *outOp = dynamic_cast<OutputOperand*>(node->m_lhs.get());
    if (outOp != NULL)
    {
        // signal, so use <=
        m_os << node->m_lhs->m_identName.c_str() << " <= " << node->m_op->m_identName.c_str() << ";\n";
    }
    else
    {
        // variable, so use :=
        m_os << node->m_lhs->m_identName.c_str() << " := " << node->m_op->m_identName.c_str() << ";\n";
    }
    return true;
}

bool VHDLCodeGen::visit(const OpNegate *node)
{
    genIndent(m_indent);    
    m_os << node->m_lhs->m_identName.c_str() << " := -" << node->m_op->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLCodeGen::visit(const OpMul *node)
{
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := " << node->m_op1->m_identName.c_str() << " * " << node->m_op2->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLCodeGen::visit(const OpAdd *node)
{
    // VHDL code generator requires that all ADD/SUB
    // nodes do not extend the MSB themselves as
    // this is not part of the VHDL specification
    // for the + or - operator.
    if (!node->m_noExtension)
    {
        //FIXME: better error reporting.
        return false;
    }
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := " << node->m_op1->m_identName.c_str() << " + " << node->m_op2->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLCodeGen::visit(const OpSub *node)
{
    // VHDL code generator requires that all ADD/SUB
    // nodes do not extend the MSB themselves as
    // this is not part of the VHDL specification
    // for the + or - operator.
    if (!node->m_noExtension)
    {
        //FIXME: better error reporting.
        return false;
    }
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := " << node->m_op1->m_identName.c_str() << " - " << node->m_op2->m_identName.c_str() << ";\n";
    return true;
}


bool VHDLCodeGen::visit(const OpNull *node)
{
    (void)node;
    return true;
}


bool VHDLCodeGen::visit(const OpExtendLSBs *node)
{
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := ";
    m_os << node->m_op->m_identName.c_str() << " & \"";
    for(int32_t i=0; i<node->m_bits; i++)
        m_os << "0";
    m_os << "\";\n";
    return true;
}


bool VHDLCodeGen::visit(const OpExtendMSBs *node)
{
    int32_t totalOpBits = node->m_op->m_intBits + node->m_op->m_fracBits;

    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := ";
    m_os << "resize(" << node->m_op->m_identName.c_str() << "," << totalOpBits + node->m_bits << ");\n";
    return true;
}


bool VHDLCodeGen::visit(const OpRemoveLSBs *node)
{
    int32_t totalOpBits = node->m_op->m_intBits + node->m_op->m_fracBits;

    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := ";
    m_os << node->m_op->m_identName.c_str() << "(" << totalOpBits-1 << " downto " << node->m_bits << "); -- remove " << node->m_bits << " LSBs\n";
    return true;
}


bool VHDLCodeGen::visit(const OpRemoveMSBs *node)
{
    int32_t totalOpBits = node->m_op->m_intBits + node->m_op->m_fracBits;

    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := ";
    m_os << node->m_op->m_identName << "(" << totalOpBits - node->m_bits - 1 << " downto 0); -- remove " << node->m_bits << " MSBs\n";
    return true;
}

bool VHDLCodeGen::visit(const OpReinterpret *node)
{
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := ";
    m_os << node->m_op->m_identName << ";\n";
    return true;
}


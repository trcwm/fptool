/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  VHDL code generator that generates
                code using REAL data types for
                model verification.

                It accepts the following nodes:
                * ADD,SUB,MUL,CSDMUL and TRUNCATE.

                As such, it can only be run on the SSA
                that is generated before _any_
                transforms/passes have been applied!

*/

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "logging.h"
#include "vhdlrealgen.h"


using namespace SSA;

VHDLRealGen::VHDLRealGen(std::ostream &os, Program &ssa) :
    m_os(os), m_ssa(&ssa), m_indent(0)
{

}

bool VHDLRealGen::execute()
{
    doLog(LOG_INFO, "-----------------------\n");
    doLog(LOG_INFO, "  Running VHDLRealGen\n");
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

void VHDLRealGen::genIndent(uint32_t indent)
{
    for(uint32_t i=0; i<indent; i++)
    {
        m_os << " ";
    }
}

void VHDLRealGen::genProcessHeader(uint32_t indent)
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
            m_os << " : REAL;  --";
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
            m_os << " : REAL;  --";
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
            m_os << " : REAL;  --";
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


bool VHDLRealGen::visit(const OpAssign *node)
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

bool VHDLRealGen::visit(const OpNegate *node)
{
    genIndent(m_indent);    
    m_os << node->m_lhs->m_identName.c_str() << " := -" << node->m_op->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLRealGen::visit(const OpMul *node)
{
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := " << node->m_op1->m_identName.c_str() << " * " << node->m_op2->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLRealGen::visit(const OpAdd *node)
{    
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := " << node->m_op1->m_identName.c_str() << " + " << node->m_op2->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLRealGen::visit(const OpSub *node)
{
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := " << node->m_op1->m_identName.c_str() << " - " << node->m_op2->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLRealGen::visit(const OpCSDMul *node)
{
    genIndent(m_indent);

    // a VHDL float should always have a decimal point
    // but ostream won't automatically generate one
    // if the value is an integer!

    std::stringstream ss;
    ss << node->m_csd.value;
    if (ss.str().find('.') == std::string::npos)
    {
        // no decimal point found, so we add ".0"
        // to make sure VHDL doesn't choke.
        ss << ".0";
    }

    m_os << node->m_lhs->m_identName.c_str() << " := " << ss.str() << " * " << node->m_op->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLRealGen::visit(const OpTruncate *node)
{
    // we simply ignore a truncate command and pass the
    // argument on directly.
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := " << node->m_op->m_identName.c_str() << ";\n";
    return true;
}

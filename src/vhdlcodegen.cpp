/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  VHDL code generator

*/

#include "logging.h"
#include <algorithm>
#include "ssaevaluator.h"
#include "vhdlcodegen.h"

using namespace SSA;

VHDLCodeGen::VHDLCodeGen(std::ostream &os, Program &ssa, bool genTestbench) :
    m_os(os), m_ssa(&ssa), m_indent(0), m_genTestbench(genTestbench)
{

}

bool VHDLCodeGen::execute()
{
    doLog(LOG_INFO, "-----------------------\n");
    doLog(LOG_INFO, "  Running VHDLCodeGen\n");
    doLog(LOG_INFO, "-----------------------\n");

    m_indent = 2;

    m_os << m_prolog;

    if (m_genTestbench)
    {
        genTestbenchHeader();
    }
    else
    {
        genEntity();

        m_os << "architecture rtl of fptool is\n";

        genArchitectureSignals();

        m_os << "begin\n";
    }

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

    genClockedProcess();

    m_os << m_epilog;

    if (m_genTestbench)
    {
        genTestbenchFooter();
    }
    else
    {
        m_os << "end rtl;\n";
    }

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

#if 0
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

#endif
    // generate process header with sensitivity list

    genIndent(indent);
    m_os << "proc_comb: process(";

    bool isFirst = true;
    for(auto operand : m_ssa->m_operands)
    {
        InputOperand *op  = dynamic_cast<InputOperand*>(operand.get());
        RegOperand *regOp = dynamic_cast<RegOperand*>(operand.get());
        if (op != NULL)
        {
            if (!isFirst)
                m_os << ",";
            m_os << op->m_identName.c_str();
            isFirst = false;
        }
        else if (regOp != NULL)
        {
            if (!isFirst)
                m_os << ",";
            m_os << regOp->m_identName.c_str();
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


void VHDLCodeGen::genClockedProcess()
{
    doLog(LOG_INFO, "-- generating clocked process\n");

    m_os << "\n";
    m_os << "  proc_clk: process(clk, rst_n)\n";
    m_os << "  begin\n";
    m_os << "    if (rising_edge(clk)) then\n";
    m_os << "      if (rst_n = '0') then\n";

    // generate reset value for registered signals
    for(auto operand : m_ssa->m_operands)
    {
        RegOperand *regOp = dynamic_cast<RegOperand*>(operand.get());
        if (regOp != NULL)
        {
            genIndent(m_indent);
            m_os << "      " << regOp->m_identName.c_str() << " <= (others=>'0');\n";
        }
    }
    m_os << "      else\n";
    // generate update assignmet for registered signals
    for(auto operand : m_ssa->m_operands)
    {
        RegOperand *regOp = dynamic_cast<RegOperand*>(operand.get());
        if (regOp != NULL)
        {
            genIndent(m_indent);
            m_os << "      " << regOp->m_identName.c_str() << " <= " << regOp->m_identName.c_str() << "_next;\n";
        }
    }
    m_os << "      end if;\n";
    m_os << "    end if;\n";
    m_os << "  end process proc_clk;\n";
}

void VHDLCodeGen::genEntity()
{
    m_os << "entity fptool is\n";
    m_os << "  port(\n";
    m_indent += 2;

    // generate input, output and register signals of the DUT
    bool firstEntry = true;
    for(auto operand : m_ssa->m_operands)
    {
        InputOperand *inOp  = dynamic_cast<InputOperand*>(operand.get());
        OutputOperand *outOp = dynamic_cast<OutputOperand*>(operand.get());

        if (inOp != NULL)
        {
            if (!firstEntry)
            {
                m_os << ";\n";
            }
            firstEntry = false;
            genIndent(m_indent);
            m_os << inOp->m_identName.c_str() << " : in SIGNED(" << inOp->m_intBits + inOp->m_fracBits-1 << " downto 0)";
        }
        else if (outOp != NULL)
        {
            if (!firstEntry)
            {
                m_os << ";\n";
            }
            firstEntry = false;
            genIndent(m_indent);
            m_os << outOp->m_identName.c_str() << " : out SIGNED(" << outOp->m_intBits + outOp->m_fracBits-1 << " downto 0)";
        }
    }
    if (!firstEntry)
    {
        m_os << "\n";
    };
    m_indent -= 2;
    m_os << "  );\n";
    m_os << "end fptool;\n\n";
}

void VHDLCodeGen::genArchitectureSignals()
{
    // generate input, output and register signals of the DUT
    for(auto operand : m_ssa->m_operands)
    {
        //InputOperand *inOp  = dynamic_cast<InputOperand*>(operand.get());
        //OutputOperand *outOp = dynamic_cast<OutputOperand*>(operand.get());
        RegOperand *regOp = dynamic_cast<RegOperand*>(operand.get());

#if 0
        if (inOp != NULL)
        {
            genIndent(m_indent);
            m_os << "signal " << inOp->m_identName.c_str();
            m_os << " : SIGNED(" << inOp->m_intBits + inOp->m_fracBits-1 << " downto 0);  --";
            m_os << " Q(" << inOp->m_intBits << "," << inOp->m_fracBits << ");\n";
        }
        else if (outOp != NULL)
        {
            genIndent(m_indent);
            m_os << "signal " << outOp->m_identName.c_str();
            m_os << " : SIGNED(" << outOp->m_intBits + outOp->m_fracBits-1 << " downto 0);  --";
            m_os << " Q(" << outOp->m_intBits << "," << outOp->m_fracBits << ");\n";
        }
#endif
        if (regOp != NULL)
        {
            genIndent(m_indent);
            m_os << "signal " << regOp->m_identName.c_str();
            m_os << " : SIGNED(" << regOp->m_intBits + regOp->m_fracBits-1 << " downto 0);  --";
            m_os << " Q(" << regOp->m_intBits << "," << regOp->m_fracBits << ");\n";
            genIndent(m_indent);
            m_os << "signal " << regOp->m_identName.c_str() << "_next";
            m_os << " : SIGNED(" << regOp->m_intBits + regOp->m_fracBits-1 << " downto 0);  --";
            m_os << " Q(" << regOp->m_intBits << "," << regOp->m_fracBits << ");\n";
        }
    }
    m_os << "\n";
}


void VHDLCodeGen::genTestbenchHeader()
{
    doLog(LOG_INFO, "-- generating testbench header\n");
    m_os << "-- \n";
    m_os << "-- FPTOOL generated test bench\n";
    m_os << "-- \n\n";

    m_os << "library ieee;\n";
    m_os << "use ieee.std_logic_1164.all;\n";
    m_os << "use ieee.numeric_std.all;\n\n";
    m_os << "entity tb is\n";
    m_os << "end tb;\n\n";

    m_os << "architecture behavioral of tb is\n";
    m_os << "    signal sim_done : std_logic := '0';\n";

    genArchitectureSignals();

    m_os << "begin\n\n";
}

void VHDLCodeGen::genTestbenchFooter()
{
    doLog(LOG_INFO, "-- generating testbench footer\n");

    // write the stimulus process
    m_os << "\n\n";
    m_os << "  proc_stim: process\n";
    m_os << "  begin\n";

    SSA::Evaluator eval(*m_ssa);
    eval.randomizeInputValues();
    eval.runProgram();

    // set values for all inputs!
    for(auto operand : m_ssa->m_operands)
    {
        InputOperand *inOp  = dynamic_cast<SSA::InputOperand*>(operand.get());
        if (inOp != NULL)
        {
            m_os << "    " << inOp->m_identName.c_str() << " <= ";
            const fplib::SFix *value = eval.getValuePtrByName(inOp->m_identName);
            if (value == NULL)
            {
                std::stringstream ss;
                ss << "VHDLCodeGen::genTestbenchFooter cannot find input variable " << inOp->m_identName;
                throw std::runtime_error(ss.str());
            }
            m_os << "\"" << value->toBinString() << "\";\n";
        }
    }
    m_os << "    wait for 1 ns;\n";

    // check values for all outputs!
    for(auto operand : m_ssa->m_operands)
    {
        OutputOperand *outOp  = dynamic_cast<SSA::OutputOperand*>(operand.get());
        if (outOp != NULL)
        {
            m_os << "    ";
            const fplib::SFix *value = eval.getValuePtrByName(outOp->m_identName);
            if (value == NULL)
            {
                std::stringstream ss;
                ss << "VHDLCodeGen::genTestbenchFooter cannot find output variable " << outOp->m_identName;
                throw std::runtime_error(ss.str());
            }
            m_os << "assert (" << outOp->m_identName << " = ";
            m_os << "\"" << value->toBinString() << "\") report \"error: ";
            m_os << outOp->m_identName << " got \" & to_string(" << outOp->m_identName << ") & \"";
            m_os << "expected: " << value->toBinString();
            m_os << "\" severity error;\n";
        }
    }

    m_os << "    wait;\n";
    m_os << "  end process proc_stim;\n";
    m_os << "end behavioral;\n";
}

std::string VHDLCodeGen::chopHexString(const std::string &hex, int32_t intBits, int32_t fracBits)
{
    int32_t nibbles = (intBits + fracBits + 3) / 4;         // minimum number of nibbles
    std::string v;

    if (nibbles < 1)
    {
        throw std::runtime_error("VHDLCodeGen::chopHexString: literal does not contain any information!");
    }

    if (hex.size() >= static_cast<uint32_t>(nibbles))
    {
        v = hex.substr(hex.size() - nibbles);
    }
    else
    {
        throw std::runtime_error("VHDLCodeGen::chopHexString: hex string does not contain enough characters!");
    }
    v = "X\"" + v;
    return v;
}

bool VHDLCodeGen::visit(const OpAssign *node)
{
    genIndent(m_indent);
    OutputOperand *outOp = dynamic_cast<OutputOperand*>(node->m_lhs.get());
    RegOperand *regOp = dynamic_cast<RegOperand*>(node->m_lhs.get());
    if (outOp != NULL)
    {
        // output signal, so use <=
        m_os << node->m_lhs->m_identName.c_str() << " <= " << node->m_op->m_identName.c_str() << ";\n";
    }
    else if (regOp != NULL)
    {
        // register signal, so use <=
        m_os << node->m_lhs->m_identName.c_str() << "_next <= " << node->m_op->m_identName.c_str() << ";\n";
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


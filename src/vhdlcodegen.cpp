/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  VHDL code generator

*/

#include <algorithm>
#include "vhdlcodegen.h"

void VHDLCodeGen::execute()
{
    uint32_t indent = 2;
    std::ostream &os = std::cout;

    genProcessHeader(os, indent);

    indent+=2;
    auto iter = m_ssaList->begin();
    while(iter != m_ssaList->end())
    {
        // get LHS operand as output variable/signal
        // and generate the code
        SSA::operand_t lhs = getOperand(iter->var3);
        genLHS(os, lhs, indent);

        SSA::operand_t op1;
        SSA::operand_t op2;
        SSA::SSANode node = *iter;
        switch(node.operation)
        {
        case SSA::SSANode::OP_Add:
            op1 = getOperand(iter->var1);
            op2 = getOperand(iter->var2);
            os << op2.info.txt << " + " << op1.info.txt << ";\n";
            // check if the fractional bits have been
            // equalized
            if (op1.info.fracBits != op2.info.fracBits)
            {
                // warning
                std::cout << "Warning: fractional bits not equalized\n";
            }
            break;
        case SSA::SSANode::OP_Sub:
            op1 = getOperand(iter->var1);
            op2 = getOperand(iter->var2);
            os << op2.info.txt << " - " << op1.info.txt << ";\n";
            // check if the fractional bits have been
            // equalized
            if (op1.info.fracBits != op2.info.fracBits)
            {
                // warning
                std::cout << "Warning: fractional bits not equalized\n";
            }
            break;
        case SSA::SSANode::OP_Mul:
            op1 = getOperand(iter->var1);
            op2 = getOperand(iter->var2);
            os << op2.info.txt << " * " << op1.info.txt << ";\n";
            break;
        case SSA::SSANode::OP_Negate:
            op1 = getOperand(iter->var1);
            os << "-" << op1.info.txt << ";\n";
            break;
        case SSA::SSANode::OP_Assign:
            op1 = getOperand(iter->var1);
            os << op1.info.txt << ";\n";
            // check if the bits are equal
            if ((op1.info.fracBits != lhs.info.fracBits) ||
                (op1.info.intBits != lhs.info.intBits))
            {
                // warning
                std::cout << "Warning: bit-widths not equalized\n";
            }
            break;
        case SSA::SSANode::OP_Saturate:
            op1 = getOperand(iter->var1);
            break;
        case SSA::SSANode::OP_RemoveLSBs:
            op1 = getOperand(iter->var1);
            os << "removeLSBs(" << op1.info.txt << ");\n";
            break;
        case SSA::SSANode::OP_ExtendLSBs:
            op1 = getOperand(iter->var1);
            os << "extendLSBs(" << op1.info.txt << "," << node.fbits - op1.info.fracBits <<");\n";
            break;
        case SSA::SSANode::OP_RemoveMSBs:
            op1 = getOperand(iter->var1);
            os << "removeMSBs(" << op1.info.txt << ");\n";
            break;
        case SSA::SSANode::OP_ExtendMSBs:
            op1 = getOperand(iter->var1);
            os << "extendMSBs(" << op1.info.txt << ");\n";
            break;
        default:
            throw std::runtime_error("VHDLCodeGen: unknown SSA operation node");
        }
        iter++;
    }
    indent-=2;
    genIndent(os, indent);
    os << "end process;\n";
}

void VHDLCodeGen::genLHS(std::ostream &os, SSA::operand_t op, uint32_t indent)
{
    // generate code for intermediate and output
    // else error.
    if (op.type == SSA::operand_t::TypeIntermediate)
    {
        genIndent(os,indent);
        os << op.info.txt << " := ";
        return;
    }
    if (op.type == SSA::operand_t::TypeOutput)
    {
        genIndent(os,indent);
        os << op.info.txt << " <= ";
        return;
    }
    throw std::runtime_error("VHDLCodeGen::genLHS LHS operand incorrect type");
}

void VHDLCodeGen::genProcessHeader(std::ostream &os, uint32_t indent)
{
    //
    // <input signal documentation>
    // <output signal documentation>
    // proc_comb: process( <sensitivity list )
    //   <variable list>
    // begin
    //

    // generate documentation for output signals
    for(size_t i=0; i<m_ssaOperands->size(); i++)
    {
        SSA::operand_t op = m_ssaOperands->at(i);
        switch(op.type)
        {
        case SSA::operand_t::TypeOutput:
            genIndent(os, indent);
            os << "-- " << op.info.txt << " Q(" << op.info.intBits << "," << op.info.fracBits << ");\n";
            break;
        }
    }

    // generate documentation for input signals
    for(size_t i=0; i<m_ssaOperands->size(); i++)
    {
        SSA::operand_t op = m_ssaOperands->at(i);
        switch(op.type)
        {
        case SSA::operand_t::TypeInput:
            genIndent(os, indent);
            os << "-- " << op.info.txt << " Q(" << op.info.intBits << "," << op.info.fracBits << ");\n";
            break;
        }
    }

    // generate process header with sensitivity list
    genIndent(os, indent);
    os << "proc_comb: process(";
    bool isFirst = true;
    for(size_t i=0; i<m_ssaOperands->size(); i++)
    {
        SSA::operand_t op = m_ssaOperands->at(i);

        if (op.type == SSA::operand_t::TypeInput)
        {
            // prepend a comma if this is not the first in the sens. list
            if (!isFirst)
                os << ",";

            os << op.info.txt;
            isFirst = false;
        }
    }
    os << ");\n"; // terminate process header
    indent+=2;

    // write the variable list
    for(size_t i=0; i<m_ssaOperands->size(); i++)
    {
        SSA::operand_t op = m_ssaOperands->at(i);
        if (op.type == SSA::operand_t::TypeIntermediate)
        {
            genIndent(os, indent);
            os << "variable " << op.info.txt << " : SIGNED("<< op.info.intBits + op.info.fracBits-1 << " downto 0);";
            os << "  -- Q(" << op.info.intBits << "," << op.info.fracBits << ");\n";
        }
    }

    indent-=2;
    genIndent(os, indent);
    os << "begin\n";
}

void VHDLCodeGen::genIndent(std::ostream &os, uint32_t indent)
{
    for(size_t i=0; i<indent; i++)
    {
        os << " ";
    }
}



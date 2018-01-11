/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  VHDL code generator

*/

#include "logging.h"
#include <algorithm>
#include "vhdlcodegen.h"

using namespace SSA;

VHDLCodeGen::VHDLCodeGen(std::ostream &os, Program &ssa) :
    m_os(os), m_ssa(&ssa)
{
    doLog(LOG_INFO, "Running VHDLCodeGen\n");
    m_indent = 2;

    m_os << m_prolog;

    genProcessHeader(m_indent);

    m_indent += 2;
    for(auto statement : ssa.m_statements)
    {
        statement->accept(this);
    }
    m_indent-=2;
    genIndent(m_indent);
    m_os << "end process;\n";

    m_os << m_epilog;
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

#if 0
    auto iter = ssa.beginOperands();
    while(iter != ssa.endOperands())
    {

        switch(iter->type)
        {
        case operand_t::TypeOutput:
            genIndent(os, indent);
            os << "-- " << iter->info.txt << " Q(" << iter->info.intBits << "," << iter->info.fracBits << ");";
            os << "  SIGNED(" << iter->info.intBits + iter->info.fracBits-1 << " downto 0);\n";
            break;
        }
        iter++;
    }

    // generate documentation for input signals
    os << "\n";
    os << "  -- *** INPUT SIGNALS ***\n";
    iter = ssa.beginOperands();
    while(iter != ssa.endOperands())
    {

        switch(iter->type)
        {
        case operand_t::TypeInput:
            genIndent(os, indent);
            os << "-- " << iter->info.txt << " Q(" << iter->info.intBits << "," << iter->info.fracBits << ");";
            os << "  SIGNED(" << iter->info.intBits + iter->info.fracBits-1 << " downto 0);\n";
            break;
        }
        iter++;
    }
#endif

    // generate process header with sensitivity list
    m_os << "\n";
    m_os << "  -------------------\n";
    m_os << "  -- PROCESS BLOCK --\n";
    m_os << "  -------------------\n";

    genIndent(indent);
    m_os << "proc_comb: process(";
#if 0
    bool isFirst = true;
    iter = ssa.beginOperands();
    while(iter != ssa.endOperands())
    {
        if (iter->type == operand_t::TypeInput)
        {
            // prepend a comma if this is not the first in the sens. list
            if (!isFirst)
                os << ",";

            os << iter->info.txt;
            isFirst = false;
        }
        iter++;
    }
#endif
    m_os << ")\n"; // terminate process header
    m_indent+=2;

#if 0
    // write the variable list
    iter = ssa.beginOperands();
    while(iter != ssa.endOperands())
    {
        if (iter->type == operand_t::TypeIntermediate)
        {
            genIndent(os, indent);
            os << "variable " << iter->info.txt << " : SIGNED("<< iter->info.intBits + iter->info.fracBits-1 << " downto 0);";
            os << "  -- Q(" << iter->info.intBits << "," << iter->info.fracBits << ");\n";
        }
        iter++;
    }
#endif
    m_indent-=2;
    genIndent(m_indent);
    m_os << "begin\n";
}


bool VHDLCodeGen::visit(const OpAssign *node)
{
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " <= " << node->m_op->m_identName.c_str() << ";\n";
    return true;
}

bool VHDLCodeGen::visit(const OpNegate *node)
{
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " <= -" << node->m_op->m_identName.c_str() << ";\n";
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
    return true;
}


bool VHDLCodeGen::visit(const OpExtendLSBs *node)
{
    genIndent(m_indent);
    m_os << node->m_lhs->m_identName.c_str() << " := ";
    m_os << node->m_op->m_identName.c_str() << " & \"";
    for(int32_t i=0; i<node->m_bits; i++)
        m_os << "0";
    m_os << "\"\n";
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


#if 0

void VHDLCodeGen::execute(SSAObject &ssa)
{
    doLog(LOG_INFO, "Running VHDLCodeGen\n");
    uint32_t indent = 2;

    os << m_prolog;

    genProcessHeader(ssa, os, indent);

    indent+=2;
    ssa_iterator iter = ssa.begin();
    while(iter != ssa.end())
    {
        // get LHS operand as output variable/signal
        // and generate the code
        operand_t lhs = ssa.getOperand(iter->lhsIdx);
        genLHS(os, lhs, indent);

        operand_t op1;
        operand_t op2;
        uint32_t totalOpBits;

        SSANode node = *iter;
        switch(node.operation)
        {
        case SSANode::OP_Add:
            op1 = ssa.getOperand(iter->op1Idx);
            op2 = ssa.getOperand(iter->op2Idx);
            op1=handleLiteralInt(op1);
            op2=handleLiteralInt(op2);
            os << op1.info.txt << " + " << op2.info.txt << ";\n";
            break;
        case SSANode::OP_Sub:
            op1 = ssa.getOperand(iter->op1Idx);
            op2 = ssa.getOperand(iter->op2Idx);
            op1=handleLiteralInt(op1);
            op2=handleLiteralInt(op2);
            os << op1.info.txt << " - " << op2.info.txt << ";\n";
            break;
        case SSANode::OP_Mul:
            op1 = ssa.getOperand(iter->op1Idx);
            op2 = ssa.getOperand(iter->op2Idx);
            op1=handleLiteralInt(op1);
            op2=handleLiteralInt(op2);
            os << op1.info.txt << " * " << op2.info.txt << ";\n";
            break;
        case SSANode::OP_Div:
            op1 = ssa.getOperand(iter->op1Idx);
            op2 = ssa.getOperand(iter->op2Idx);
            op1=handleLiteralInt(op1);
            op2=handleLiteralInt(op2);
            os << op1.info.txt << " / " << op2.info.txt << "; -- Note: this doesn't work!\n";
            break;
        case SSANode::OP_Negate:
            op1 = ssa.getOperand(iter->op1Idx);
            op1=handleLiteralInt(op1);
            os << "-" << op1.info.txt << ";\n";
            break;
        case SSANode::OP_Assign:
            op1 = ssa.getOperand(iter->op1Idx);
            op1=handleLiteralInt(op1);
            os << op1.info.txt << ";\n";
            break;
        case SSANode::OP_Saturate:
            op1 = ssa.getOperand(iter->op1Idx);
            break;
        case SSANode::OP_RemoveLSBs:
            op1 = ssa.getOperand(iter->op1Idx);
            totalOpBits = op1.info.intBits + op1.info.fracBits;
            os << op1.info.txt << "(" << totalOpBits-1 << " downto " << iter->bits << "); -- remove " << iter->bits << " LSBs\n";
            break;
        case SSANode::OP_ExtendLSBs:
            op1 = ssa.getOperand(iter->op1Idx);
            extendLSBs(os, op1.info.txt, node.bits);
            os << ";\n";
            break;
        case SSANode::OP_RemoveMSBs:
            op1 = ssa.getOperand(iter->op1Idx);
            totalOpBits = op1.info.intBits + op1.info.fracBits;
            os << op1.info.txt << "(" << totalOpBits - iter->bits - 1<< " downto 0); -- remove " << iter->bits << " MSBs\n";
            break;
        case SSANode::OP_ExtendMSBs:
            op1 = ssa.getOperand(iter->op1Idx);
            os << "resize(" << op1.info.txt << "," << op1.info.intBits+op1.info.fracBits+node.bits << ");\n";
            break;
        case SSANode::OP_Reinterpret:
            op1 = ssa.getOperand(iter->op1Idx);
            os << op1.info.txt << "; -- reinterpret as Q(" << iter->bits << "," << iter->fbits << ");\n";
            break;
        case SSANode::OP_Truncate:
            op1 = ssa.getOperand(iter->op1Idx);
            os << op1.info.txt << "; -- truncate to Q(" << iter->bits << "," << iter->fbits << ");\n";
            break;
        default:
            throw std::runtime_error("VHDLCodeGen: unknown SSA operation node");
        }
        iter++;
    }
    indent-=2;
    genIndent(os, indent);
    os << "end process;\n";

    os << m_epilog;
}

operand_t VHDLCodeGen::handleLiteralInt(operand_t &op)
{
    if (op.type == operand_t::TypeInteger)
    {
        std::stringstream ss;
        operand_t newOp = op;
        ss << "TO_SIGNED(" << op.info.intVal << "," << op.info.intBits << ")";
        newOp.info.txt = ss.str();
        return newOp;
    }
    else
    {
        return op;
    }
}

void VHDLCodeGen::genLHS(std::ostream &os, operand_t op, uint32_t indent)
{
    // generate code for intermediate and output
    // else error.
    if (op.type == operand_t::TypeIntermediate)
    {
        genIndent(os,indent);
        os << op.info.txt << " := ";
        return;
    }
    if (op.type == operand_t::TypeOutput)
    {
        genIndent(os,indent);
        os << op.info.txt << " <= ";
        return;
    }
    throw std::runtime_error("VHDLCodeGen::genLHS LHS operand incorrect type");
}

void VHDLCodeGen::genProcessHeader(const SSAObject &ssa, std::ostream &os, uint32_t indent)
{
    //
    // <input signal documentation>
    // <output signal documentation>
    // proc_comb: process( <sensitivity list )
    //   <variable list>
    // begin
    //

    // generate documentation for output signals
    os << "  -- *** OUTPUT SIGNALS ***\n";
    auto iter = ssa.beginOperands();    
    while(iter != ssa.endOperands())
    {

        switch(iter->type)
        {
        case operand_t::TypeOutput:
            genIndent(os, indent);
            os << "-- " << iter->info.txt << " Q(" << iter->info.intBits << "," << iter->info.fracBits << ");";
            os << "  SIGNED(" << iter->info.intBits + iter->info.fracBits-1 << " downto 0);\n";
            break;
        }
        iter++;
    }

    // generate documentation for input signals
    os << "\n";
    os << "  -- *** INPUT SIGNALS ***\n";
    iter = ssa.beginOperands();
    while(iter != ssa.endOperands())
    {

        switch(iter->type)
        {
        case operand_t::TypeInput:
            genIndent(os, indent);
            os << "-- " << iter->info.txt << " Q(" << iter->info.intBits << "," << iter->info.fracBits << ");";
            os << "  SIGNED(" << iter->info.intBits + iter->info.fracBits-1 << " downto 0);\n";
            break;
        }
        iter++;
    }

    // generate process header with sensitivity list
    os << "\n";
    os << "  -------------------\n";
    os << "  -- PROCESS BLOCK --\n";
    os << "  -------------------\n";

    genIndent(os, indent);
    os << "proc_comb: process(";
    bool isFirst = true;
    iter = ssa.beginOperands();
    while(iter != ssa.endOperands())
    {
        if (iter->type == operand_t::TypeInput)
        {
            // prepend a comma if this is not the first in the sens. list
            if (!isFirst)
                os << ",";

            os << iter->info.txt;
            isFirst = false;
        }
        iter++;
    }
    os << ")\n"; // terminate process header
    indent+=2;

    // write the variable list
    iter = ssa.beginOperands();
    while(iter != ssa.endOperands())
    {
        if (iter->type == operand_t::TypeIntermediate)
        {
            genIndent(os, indent);
            os << "variable " << iter->info.txt << " : SIGNED("<< iter->info.intBits + iter->info.fracBits-1 << " downto 0);";
            os << "  -- Q(" << iter->info.intBits << "," << iter->info.fracBits << ");\n";
        }
        iter++;
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

/** extend LSBs of a signal by adding zeroes */
void VHDLCodeGen::extendLSBs(std::ostream &os, const std::string &name, uint32_t bits)
{
    // sanity check
    if (bits > 2048)
    {
        printf("VHDLCodeGen::extendLSB - error: extending more than 2048 bits!\n");
        printf("                         truncating to 2048 - your VHDL won't work.\n");
        printf("                         this is most likely due to a bug in fptool.\n");
        bits = 2048;
    }

    os << name << " & \"";
    for(uint32_t i=0; i<bits; i++)
        os << "0";
    os << "\"";
}

#endif

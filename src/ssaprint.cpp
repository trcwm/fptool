
#include "ssaprint.h"

using namespace SSA;

bool SSA::Printer::print(const Program &program, std::ostream &s, bool printLHSPrecision)
{
    Printer printer(s, printLHSPrecision);
    for(auto smnt : program.m_statements)
    {
        if (!smnt->accept(&printer))
        {
            return false;
        }
    }
    return true;
}

bool SSA::Printer::visit(const OpAdd *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := ADD " << node->m_op1->m_identName.c_str();
    m_s << "," << node->m_op2->m_identName.c_str() << "\n";
    return true;
}

bool SSA::Printer::visit(const OpSub *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := SUB " << node->m_op1->m_identName.c_str();
    m_s << "," << node->m_op2->m_identName.c_str() << "\n";
    return true;
}

bool SSA::Printer::visit(const OpMul *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := MUL " << node->m_op1->m_identName.c_str();
    m_s << "," << node->m_op2->m_identName.c_str() << "\n";
    return true;
}

bool SSA::Printer::visit(const OpCSDMul *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := CSDMUL " << node->m_csdName.c_str();
    m_s << "," << node->m_op->m_identName.c_str() << "\n";
    return true;
}

bool SSA::Printer::visit(const OpTruncate *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := TRUNC(" << node->m_op->m_identName.c_str();
    m_s << "," << node->m_intBits;
    m_s << "," << node->m_fracBits << ")\n";
    return true;
}


bool SSA::Printer::visit(const OpNegate *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := -" << node->m_op->m_identName.c_str();
    m_s << "\n";
    return true;
}


bool SSA::Printer::visit(const OpAssign *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := " << node->m_op->m_identName.c_str();
    m_s << "\n";
    return true;
}

bool SSA::Printer::visit(const OpReinterpret *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := REINTERPRET(" << node->m_op->m_identName.c_str();
    m_s << "," << node->m_intBits;
    m_s << "," << node->m_fracBits << ")\n";
    return true;
}

bool SSA::Printer::visit(const OperationSingle *node)
{
    // we should never see these nodes as they meant to
    // be base classes.
    return false;
}

bool SSA::Printer::visit(const OperationDual *node)
{
    // we should never see these nodes as they meant to
    // be base classes.
    return false;
}

bool SSA::Printer::visit(const OpPatchBlock *node)
{
    // special patch node that holds
    // additional instructions that need
    // to be patched/integrated into the
    // top-level instruction stream
    m_s << "** PATCH BLOCK BEGIN **\n";
    for(auto smnt : node->m_statements)
    {
        if (!smnt->accept(this))
        {
            return false;
        }
    }
    m_s << "** PATCH BLOCK END **\n";
    return true;
}

bool SSA::Printer::visit(const OpExtendLSBs *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := EXTENDLSBS(" << node->m_op->m_identName.c_str();
    m_s << "," << node->m_bits << ")\n";
    return true;
}

bool SSA::Printer::visit(const OpExtendMSBs *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := EXTENDMSBS(" << node->m_op->m_identName.c_str();
    m_s << "," << node->m_bits << ")\n";
    return true;
}

bool SSA::Printer::visit(const OpRemoveLSBs *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := REMOVELSBS(" << node->m_op->m_identName.c_str();
    m_s << "," << node->m_bits << ")\n";
    return true;
}

bool SSA::Printer::visit(const OpRemoveMSBs *node)
{
    if (m_printLHSPrecision)
    {
        m_s << "Q(" << node->m_lhs->m_intBits;
        m_s << "," << node->m_lhs->m_fracBits;
        m_s << ")\t";
    }
    m_s << node->m_lhs->m_identName.c_str() << " := REMOVEMSBS(" << node->m_op->m_identName.c_str();
    m_s << "," << node->m_bits << ")\n";
    return true;
}

bool SSA::Printer::visit(const OpNull *node)
{
    m_s << "NOP\n";
    return true;
}


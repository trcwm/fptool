
#include "ssaprint.h"

using namespace SSA;

bool SSA::Printer::print(const Program &program, std::ostream &s)
{
    Printer printer(s);
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
    m_s << node->m_lhs->m_identName.c_str() << " := ADD " << node->m_op1->m_identName.c_str();
    m_s << "," << node->m_op2->m_identName.c_str() << "\n";
    return true;
}

bool SSA::Printer::visit(const OpSub *node)
{
    m_s << node->m_lhs->m_identName.c_str() << " := SUB " << node->m_op1->m_identName.c_str();
    m_s << "," << node->m_op2->m_identName.c_str() << "\n";
    return true;
}

bool SSA::Printer::visit(const OpMul *node)
{
    m_s << node->m_lhs->m_identName.c_str() << " := MUL " << node->m_op1->m_identName.c_str();
    m_s << "," << node->m_op2->m_identName.c_str() << "\n";
    return true;
}

bool SSA::Printer::visit(const OpTruncate *node)
{
    m_s << node->m_lhs->m_identName.c_str() << " := TRUNC(" << node->m_op->m_identName.c_str();
    m_s << ",?,?)\n";
    return true;
}


bool SSA::Printer::visit(const OpAssign *node)
{
    m_s << node->m_lhs->m_identName.c_str() << " := " << node->m_op->m_identName.c_str();
    m_s << "\n";
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

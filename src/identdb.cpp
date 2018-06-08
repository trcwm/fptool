
#include <sstream>
#include "identdb.h"

void SymbolTable::dump(std::ostream &os)
{
    std::stringstream ss;

    auto citer = m_identifiers.cbegin();
    while(citer != m_identifiers.end())
    {
        ss << "Name: " << citer->second.m_name;

        int32_t padding = 12 - citer->second.m_name.size();
        if (padding>0)
        {
            for(int32_t i=0; i<padding; i++)
            {
                ss << " ";
            }
        }

        switch (citer->second.m_type)
        {
        case SymbolInfo::T_INPUT:   ss << "INPUT   "; break;
        case SymbolInfo::T_OUTPUT:  ss << "OUTPUT  "; break;
        case SymbolInfo::T_CSD:     ss << "CSD     "; break;
        case SymbolInfo::T_REG:     ss << "REG     "; break;
        case SymbolInfo::T_TMP:     ss << "TMP     "; break;
        default:
            os << "UNKNOWN "; break;
        }
        ss << "Q(" << citer->second.m_intBits;
        ss << ","  << citer->second.m_fracBits << ")\n";
        citer++;
    }
    os << ss.str();
}


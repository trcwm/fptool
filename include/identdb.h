/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Symbol table

  Author: Niels A. Moseley 2018

*/

#ifndef __identdb_h
#define __identdb_h

#include <map>
#include <string>
#include <ostream>
#include <stdint.h>
#include "fplib.h"

/** identifier types for type checking */
class SymbolInfo
{
public:
    SymbolInfo() : m_type(T_UNINIT),
        m_intBits(0),
        m_fracBits(0)
    {

    }

    enum type_t
    {
        T_UNINIT  = 0,      ///< uninitialized variable
        T_INPUT   = 1,      ///< input variable
        T_OUTPUT  = 2,      ///< output variable
        T_CSD     = 3,      ///< CSD multiplication constant
        T_REG     = 4,      ///< register variable
        T_TMP     = 5,      ///< temporary/intermediate variable
        T_NOTFOUND = 9999   ///< special return variable for lookups: identifier not found.
    };

    std::string m_name; ///< symbol name
    type_t  m_type;     ///< type of symbol

    int32_t m_intBits;  ///< input or register precision (integer bits).
    int32_t m_fracBits; ///< input or register precision (fractional bits).

    fplib::SFix m_min;  ///< minimum value
    fplib::SFix m_max;  ///< maximum value
};


class SymbolTable
{
public:
    SymbolTable() {}
    virtual ~SymbolTable() {}

    /****************************************
      Identifier type checking
    ****************************************/

    /** check if an identifier is of a specific type.
        @return true if the identifier is of a specific type.
    */
    bool identIsType(const std::string &ident, SymbolInfo::type_t t) const
    {
        auto iter = m_identifiers.find(ident);
        if (iter != m_identifiers.end())
        {
            if (iter->second.m_type == t)
            {
                // identifier found and of type
                return true;
            }
        }
        return false;   // identifier not found or wrong type.
    }

    /** add identifier to database.
        @return true success, false if ident already exists.
    */
    bool addIdentifier(const std::string &ident, SymbolInfo::type_t t,
                       int32_t m_intBits = 0, int32_t m_fracBits = 0)
    {
        if (!hasIdentifier(ident))
        {
            SymbolInfo it;
            it.m_name = ident;
            it.m_type = t;
            it.m_intBits = m_intBits;
            it.m_fracBits = m_fracBits;

            m_identifiers[ident] = it;
            return true;
        }
        else
        {
            return false;
        }
    }

    /** @return true if the identifier is present in the
        database.
    */
    bool hasIdentifier(const std::string &ident) const
    {
        auto iter = m_identifiers.find(ident);
        return (iter != m_identifiers.end());
    }

    /** clear the database */
    void clear()
    {
        m_identifiers.clear();
    }

    SymbolInfo::type_t getType(const std::string &ident) const
    {
        auto iter = m_identifiers.find(ident);
        if (iter != m_identifiers.end())
        {
            return iter->second.m_type;
        }
        return SymbolInfo::T_NOTFOUND;
    }

    SymbolInfo* getIdentifiedByName(const std::string &name)
    {
        auto iter = m_identifiers.find(name);
        if (iter != m_identifiers.end())
        {
            return &(iter->second);
        }
        else
        {
            return NULL;
        }
    }

    /** dump the symbol table to an output stream */
    void dump(std::ostream &os);

    std::map<std::string, SymbolInfo> m_identifiers;
};

#endif

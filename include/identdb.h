/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Identifier database class

  Author: Niels A. Moseley 2018

*/

#ifndef __identdb_h
#define __identdb_h

#include <map>
#include <string>
#include <stdint.h>

class IdentDB
{
public:
    IdentDB() {}
    virtual ~IdentDB() {}

    /** identifier types for type checking */
    struct info_t
    {
        info_t() : m_type(T_UNKNOWN),
            m_intBits(0),
            m_fracBits(0)
        {

        }

        enum type_t {
            T_UNKNOWN = 0,
            T_INPUT   = 1,
            T_OUTPUT  = 2,
            T_CSD     = 3,
            T_REG     = 4
        } m_type;

        int32_t m_intBits;
        int32_t m_fracBits;
    };

    /****************************************
      Identifier type checking
    ****************************************/

    /** check if an identifier is of a specific type.
        @return true if the identifier is of a specific type.
    */
    bool identIsType(const std::string &ident, info_t::type_t t)
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
    bool addIdentifier(const std::string &ident, info_t::type_t t,
                       int32_t m_intBits = 0, int32_t m_fracBits = 0)
    {
        info_t it;
        it.m_type = t;
        it.m_intBits = m_intBits;
        it.m_fracBits = m_fracBits;

        // check if identifier already exists
        auto iter = m_identifiers.find(ident);
        if (iter == m_identifiers.end())
        {
            m_identifiers[ident] = it;
            return true;
        }
        return false;
    }

    std::map<std::string, info_t> m_identifiers;
};

#endif

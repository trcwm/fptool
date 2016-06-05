/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  A base class that defines an SSA
                compiler pass interface

*/

#ifndef ssapass_h
#define ssapass_h

#include <stdexcept>
#include <sstream>
#include "ssa.h"

/** Generic compiler pass interface that processes an SSA tree.
    Errors should be handled through std::runtime_error exceptions.
*/
class SSAPass
{
public:
    void process(SSA::ssaList_t &ssaList, SSA::ssaOperands_t &ssaOperands)
    {
        m_ssaList = &ssaList;
        m_ssaOperands = &ssaOperands;
        execute();
    }

protected:
    virtual void execute() = 0;

    /** lookup a variable in t he operand list.
        if none exists, throw an exception.
    */
    SSA::operand_t getOperand(size_t index)
    {
        if (index >= m_ssaOperands->size())
            throw std::runtime_error("Operand out of bounds");

        return m_ssaOperands->at(index);
    }

    /** create a new intermediate variable and return
        its index into the operand array.

        Note: this whole thing assumes that the operand list will only
        grow. To "kill" nodes, set their type to TypeRemoved.
    */
    uint32_t createIntermediate(int32_t intBits, int32_t fracBits)
    {
        const std::string m_tempPrefix = "tmp";
        SSA::operand_t op;
        op.type = SSA::operand_t::TypeIntermediate;
        op.info.intBits = intBits;
        op.info.fracBits = fracBits;

        uint32_t index = m_ssaOperands->size();

        std::ostringstream ss;
        ss << m_tempPrefix << index;
        op.info.txt = ss.str();

        m_ssaOperands->push_back(op);
        return index;
    }

    /** mark an operand as removed */
    void removeOperand(uint32_t index)
    {
        if (index < m_ssaOperands->size())
        {
            m_ssaOperands->operator [](index).type = SSA::operand_t::TypeRemoved;
        }
        else
        {
            throw std::runtime_error("removeOperand out-of-bounds");
        }
    }

    SSA::ssaList_t      *m_ssaList;
    SSA::ssaOperands_t  *m_ssaOperands;
};

#endif



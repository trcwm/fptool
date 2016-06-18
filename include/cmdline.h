/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  command line option parsing

  Author: Niels A. Moseley

*/

#ifndef cmdline_h
#define cmdline_h

#include <map>
#include <string>
#include "stdint.h"

/** A flexible command line option parser.
    Usage:

    CmdLine cmdline("fod", true);

*/
class CmdLine
{
public:
    CmdLine(const std::string &acceptedOptions, bool mainArgRequired = true);

    /** parse the command line */
    bool parseOptions(int32_t argc, char *argv[]);

    /** get the value of an option */
    bool getOption(char opt, std::string &val) const
    {
        auto iter = m_options.find(opt);
        if (iter!=m_options.end())
        {
            val = iter->second;
            return true;
        }
        else
        {
            return false;
        }
    }

    /** get the main argument */
    std::string getMainArg() const
    {
        return m_mainArg;
    }

protected:
    std::map<char, std::string> m_options;
    std::string m_acceptedOptions;
    std::string m_mainArg;
    bool m_mainArgRequired;
};

#endif

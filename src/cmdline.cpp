/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  command line option parsing

  Author: Niels A. Moseley

*/

#include "logging.h"
#include "cmdline.h"

CmdLine::CmdLine(const std::string &acceptedOptions, bool mainArgRequired)
    : m_acceptedOptions(acceptedOptions), m_mainArgRequired(mainArgRequired)
{

}

bool CmdLine::parseOptions(int32_t argc, char *argv[])
{
    typedef enum {S_start, S_optionarg} state_t;

    state_t state = S_start;
    int32_t idx = 1;

    char option = 0;
    while(idx < argc)
    {
        const char *ptr = argv[idx];
        switch(state)
        {
        case S_start:
            if (*ptr == '-')
            {
                if (m_acceptedOptions.find(ptr[1]) != std::string::npos)
                {
                    // option is accepted
                    option = ptr[1];
                    state = S_optionarg;
                }
                else
                {
                    // error, invalid option
                    doLog(LOG_ERROR, "Unknown option %s\n", ptr);
                    return false;
                }
                idx++;
            }
            else
            {
                // main arguments
                if (m_mainArg != std::string(""))
                {
                    doLog(LOG_ERROR, "Multiple main arguments (%s)\n", ptr);
                    return false;
                }
                m_mainArg = ptr;
                idx++;
            }
            break;
        case S_optionarg:
            // retrieve the option
            m_options.insert(std::pair<char, std::string>(option,ptr));
            state = S_start;
            idx++;
            break;
        default:
            doLog(LOG_ERROR, "Unknown command line parse state\n");
            return false;
        }
    }

    if ((m_mainArgRequired) && (m_mainArg == std::string("")))
    {
        doLog(LOG_ERROR, "Main argument not found!\n");
        return false;
    }

    return true;
}

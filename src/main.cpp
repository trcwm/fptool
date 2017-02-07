/*

  FPTOOL - a fixed-point math to VHDL generation tool

*/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "logging.h"
#include "cmdline.h"
#include "reader.h"
#include "tokenizer.h"
#include "parser.h"
#include "ssa.h"
#include "csd.h"
#include "pass_addsub.h"
#include "pass_csdmul.h"
#include "pass_clean.h"
#include "vhdlcodegen.h"

#define __VERSION__ "0.1a"

int main(int argc, char *argv[])
{
    CmdLine cmdline("o");

    setDebugging();

    printf("FPTOOL version "__VERSION__" compiled on "__DATE__"\n\n");
    if (!cmdline.parseOptions(argc, argv))
    {

        printf("\nUsage: fptool <source.fp>\n\n");
        return 1;
    }
    else
    {        
        Reader* reader = Reader::open(cmdline.getMainArg().c_str());
        if (reader == 0)
        {
            printf("Error opening file! %s\n", cmdline.getMainArg().c_str());
            return 1;
        }

        std::string outfile;
        std::ofstream outstream;
        if (cmdline.getOption('o', outfile))
        {
            outstream = std::ofstream(outfile, std::ios::out);
            doLog(LOG_DEBUG, "output file: %s\n", outfile.c_str());
        }

        Tokenizer tokenizer;
        std::vector<token_t> tokens;
        tokenizer.process(reader, tokens);
        tokenizer.dumpTokens(std::cout, tokens);
        delete reader;

        Parser parse;
        statements_t statements;
        if (parse.process(tokens, statements))
        {
            doLog(LOG_INFO, "Parse OK!\n");

            SSACreator ssaCreator;
            SSAObject ssa;
            if (!ssaCreator.process(statements, ssa))
            {
                doLog(LOG_ERROR, "Error producing SSA: %s\n", ssaCreator.getLastError().c_str());
            }

            //ssa.dumpStatements(std::cout);

            PassCSDMul csdmul;
            csdmul.process(ssa);

            PassAddSub  addsub;
            addsub.process(ssa);

            PassClean  clean;
            clean.process(ssa);

            if (outstream.bad())
            {
                VHDLCodeGen codegen(std::cout);
                codegen.process(ssa);
            }
            else
            {
                VHDLCodeGen codegen(outstream);
                codegen.setEpilog("");
                codegen.process(ssa);
            }

        }
        else
        {
            doLog(LOG_ERROR, "Parse Failed!\n");
            doLog(LOG_ERROR, "Line %d pos %d: %s\n", parse.getLastErrorPos().line+1,
                  parse.getLastErrorPos().pos+1,
                  parse.getLastError().c_str());
        }
    }

    return 0;
}


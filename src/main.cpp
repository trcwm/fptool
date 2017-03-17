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
#include "astgraphviz.h"

#define __VERSION__ "0.1a"

int main(int argc, char *argv[])
{
    CmdLine cmdline("og");

    setDebugging();

    printf("FPTOOL version "__VERSION__" compiled on "__DATE__"\n\n");
    if (!cmdline.parseOptions(argc, argv))
    {

        printf("\nUsage: fptool <source.fp>\n\n");
        printf("options: \n");
        printf("  -o <outputfile>    Output file for VHDL code.\n");
        printf("  -g <graphvizfile>  Output file for Graphviz/dot program visualisation.\n");
        printf("\n\n");
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

        std::string graphvizFilename;
        std::ofstream graphvizStream;
        if (cmdline.getOption('g', graphvizFilename))
        {
            graphvizStream = std::ofstream(graphvizFilename, std::ios::out);
            doLog(LOG_DEBUG, "graphviz file: %s\n", graphvizFilename.c_str());
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

            // dump the AST
            ASTDumpVisitor ASTdumper(std::cout);
            for(uint32_t i=0; i<statements.size(); i++)
            {
                std::cout << "Statement " << i+1 << ":\n";
                ASTdumper.visit(statements[i]);
                std::cout << "\n";
            }

            // dump the AST using graphviz
            if (graphvizStream.is_open())
            {
                AST2Graphviz graphviz(graphvizStream);
                graphviz.writeProlog();
                for(uint32_t i=0; i<statements.size(); i++)
                {
                    graphviz.addStatement(statements[i]);
                }
                graphviz.writeEpilog();
                graphvizStream.close();
            }

            SSACreator ssaCreator;
            SSAObject ssa;
            if (!ssaCreator.process(statements, ssa))
            {
                doLog(LOG_ERROR, "Error producing SSA: %s\n", ssaCreator.getLastError().c_str());
            }

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


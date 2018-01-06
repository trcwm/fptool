/*

  FPTOOL - a fixed-point math to VHDL generation tool

*/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "logging.h"
#include "cmdline.h"
#include "reader.h"
#include "tokenizer.h"
#include "parser.h"
#include "ssa.h"
#include "ssacreator.h"
#include "ssaprint.h"

//#include "ssaevaluator.h"
#include "csd.h"
#include "pass_addsub.h"
#include "pass_truncate.h"
#include "pass_csdmul.h"
#include "pass_clean.h"
#include "vhdlcodegen.h"
#include "astgraphviz.h"

#define __FPTOOLVERSION__ "0.1a"

int main(int argc, char *argv[])
{
    bool verbose = false;
    CmdLine cmdline("ogL","dV");

    printf("FPTOOL version " __FPTOOLVERSION__ " compiled on " __DATE__ "\n\n");
    if (!cmdline.parseOptions(argc, argv))
    {

        printf("\nUsage: fptool <source.fp>\n\n");
        printf("options: \n");
        printf("  -o <outputfile>    Output file for VHDL code.\n");
        printf("  -g <graphvizfile>  Output file for Graphviz/dot program visualisation.\n");
        printf("  -L <logfile>       Write output log to file.\n");
        printf("  -d                 Enable debug output.\n");
        printf("  -V                 Enable verbose output.\n");
        printf("\n\n");
        return 1;
    }
    else
    {
        if (cmdline.hasOption('d'))
        {
            setDebugging();
        }

        if (cmdline.hasOption('V'))
        {
            verbose = true;
        }

        std::string logfile;
        if (cmdline.getOption('L', logfile))
        {
            doLog(LOG_INFO, "Logging to file: %s\n", logfile.c_str());
            setLogFile(logfile.c_str());
        }

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
            outstream.open(outfile, std::ofstream::out);
            doLog(LOG_INFO, "output file: %s\n", outfile.c_str());
        }

        std::string graphvizFilename;
        std::ofstream graphvizStream;
        if (cmdline.getOption('g', graphvizFilename))
        {
            graphvizStream.open(graphvizFilename, std::ofstream::out);
            doLog(LOG_INFO, "Graphviz/dot file: %s\n", graphvizFilename.c_str());
        }

        Tokenizer tokenizer;
        std::vector<token_t> tokens;
        tokenizer.process(reader, tokens);

        if (cmdline.hasOption('d'))
        {
            tokenizer.dumpTokens(std::cout, tokens);
        }

        delete reader;

        Parser parse;
        AST::Statements statements;
        if (parse.process(tokens, statements))
        {
            doLog(LOG_INFO, "Parse OK!\n");

            if (cmdline.hasOption('d'))
            {
                // dump the AST
                AST::DumpVisitor ASTdumper(std::cout);
                for(ASTNode *node : statements.m_statements)
                {
                    if (node != NULL)
                    {
                        node->accept(&ASTdumper);
                    }
                }
            }

            // dump the AST using graphviz
            if (graphvizStream.is_open())
            {
                AST2Graphviz graphviz(graphvizStream, true);
                graphviz.writeProlog();
                for(ASTNode *node : statements.m_statements)
                {
                    graphviz.addStatement(node);
                }
                graphviz.writeEpilog();
                graphvizStream.close();
            }

            SSA::Creator ssaCreator;
            SSA::Program ssa;
            if (!ssaCreator.process(statements, ssa))
            {
                doLog(LOG_ERROR, "Error producing SSA: %s\n", ssaCreator.getLastError().c_str());
            }

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }
#if 0
            SSAObject referenceSSA = ssa;

            SSAEvaluator eval;
            // set all inputs for evaluation
            auto iter = ssa.beginOperands();
            uint32_t opIndex = 0;
            while(iter != ssa.endOperands())
            {
                if (iter->type == operand_t::TypeInput)
                {
                    doLog(LOG_INFO,"Setting input (var index=%d) %s to zero\n", opIndex, iter->info.txt.c_str());
                    eval.setInputVariable(opIndex, fplib::SFix(iter->info.intBits, iter->info.fracBits));
                }
                if (iter->type == operand_t::TypeCSD)
                {
                    doLog(LOG_INFO,"Setting CSD (var index=%d) %s to %f\n", opIndex, iter->info.txt.c_str()
                          ,iter->info.csdFloat);
                    eval.createCSDConstant(opIndex, iter->info.csd);
                }
                iter++;
                opIndex++;
            }
            eval.process(ssa);

            iter = ssa.beginOperands();
            opIndex = 0;
            while(iter != ssa.endOperands())
            {
                if (iter->type == operand_t::TypeOutput)
                {
                    fplib::SFix output;
                    if (eval.getValue(opIndex, output))
                    {
                        doLog(LOG_INFO, "Output (index=%d) %s: Q(%d,%d) %s\n",
                              opIndex,
                              iter->info.txt.c_str(),
                              output.intBits(),
                              output.fracBits(),
                              output.toHexString().c_str());
                    }
                }
                iter++;
                opIndex++;
            }
#endif

            // ------------------------------------------------------------
            // -- CSD PASS
            // ------------------------------------------------------------
            SSA::PassCSDMul::execute(ssa);

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

#if 0
            SSAEvaluator eval2;
            // set all inputs for evaluation
            iter = ssa.beginOperands();
            opIndex = 0;
            while(iter != ssa.endOperands())
            {
                if (iter->type == operand_t::TypeInput)
                {
                    doLog(LOG_INFO,"Setting input (var index=%d) %s to zero\n", opIndex, iter->info.txt.c_str());
                    eval2.setInputVariable(opIndex, fplib::SFix(iter->info.intBits, iter->info.fracBits));
                }
                if (iter->type == operand_t::TypeCSD)
                {
                    doLog(LOG_INFO,"Setting CSD (var index=%d) %s to %f\n", opIndex, iter->info.txt.c_str()
                          ,iter->info.csdFloat);
                    eval2.createCSDConstant(opIndex, iter->info.csd);
                }
                iter++;
                opIndex++;
            }
            eval2.process(ssa);

            iter = ssa.beginOperands();
            opIndex = 0;
            while(iter != ssa.endOperands())
            {
                if (iter->type == operand_t::TypeOutput)
                {
                    fplib::SFix output;
                    if (eval2.getValue(opIndex, output))
                    {
                        doLog(LOG_INFO, "Output (index=%d) %s: Q(%d,%d) %s\n",
                            opIndex,
                            iter->info.txt.c_str(),
                            output.intBits(),
                            output.fracBits(),
                            output.toHexString().c_str());
                    }
                }
                iter++;
                opIndex++;
            }



            if (!fuzzer(referenceSSA, ssa, 1))
            {
                doLog(LOG_ERROR, "Fuzzer reported an error!\n");
                return 1;
            }
#endif


            // ------------------------------------------------------------
            // -- ADDSUB PASS
            // ------------------------------------------------------------
            SSA::PassAddSub::execute(ssa);

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- TRUNCATE PASS
            // ------------------------------------------------------------
            SSA::PassTruncate::execute(ssa);

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- CLEAN PASS
            // ------------------------------------------------------------
            SSA::PassClean::execute(ssa);

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- VHDL code generation
            // ------------------------------------------------------------
            if (outstream.bad())
            {
                SSA::VHDLCodeGen codegen(std::cout, ssa);
                //codegen.process(ssa);
            }
            else
            {
                SSA::VHDLCodeGen codegen(outstream, ssa);
                //codegen.setEpilog("");
                //codegen.process(ssa);
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

    closeLogFile();

    return 0;
}


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

#include "ssaevaluator.h"
#include "csd.h"
#include "pass_addsub.h"
#include "pass_regtrunc.h"
#include "pass_truncate.h"
#include "pass_csdmul.h"
#include "pass_clean.h"
#include "pass_removeoperands.h"
#include "vhdlcodegen.h"
#include "vhdlrealgen.h"
#include "astgraphviz.h"

#define __FPTOOLVERSION__ "0.1a"


int main(int argc, char *argv[])
{
    bool verbose = false;
    CmdLine cmdline("ogL","dVr");

    printf("FPTOOL version " __FPTOOLVERSION__ " compiled on " __DATE__ "\n\n");
    if (!cmdline.parseOptions(argc, argv))
    {
        printf("\nUsage: fptool <source.fp>\n\n");
        printf("options: \n");
        printf("  -o <outputfile>    Output file for VHDL code.\n");
        printf("  -g <graphvizfile>  Output file for Graphviz/dot program visualisation.\n");
        printf("  -L <logfile>       Write output log to file.\n");
        printf("  -r                 Generate REAL-based VHDL code.\n");
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
        SymbolTable         symbolTable;
        if (parse.process(tokens, statements, symbolTable))
        {
            doLog(LOG_INFO, "Parse OK!\n");

            if (cmdline.hasOption('d'))
            {
                // dump the AST
                AST::DumpVisitor ASTdumper(std::cout);
                for(AST::ASTNodeBase *node : statements.m_statements)
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
                for(AST::ASTNodeBase *node : statements.m_statements)
                {
                    graphviz.addStatement(node);
                }
                graphviz.writeEpilog();
                graphvizStream.close();
            }

            SSA::Creator ssaCreator;
            SSA::Program ssa;

            if (!ssaCreator.process(statements, symbolTable, ssa))
            {
                doLog(LOG_ERROR, "Error producing SSA: %s\n", ssaCreator.getLastError().c_str());
                return 1;
            }

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // if we require REAL-based VHDL, we should output if now
            // before the transforms add instructions that are not
            // supported by the VHDL generator
            if (cmdline.hasOption('r'))
            {
                // ------------------------------------------------------------
                // -- VHDL code generation
                // ------------------------------------------------------------
                if (outstream.bad())
                {
                    if (!SSA::VHDLRealGen::generateCode(std::cout, ssa))
                    {
                        doLog(LOG_ERROR, "Error generating VHDL code!\n");
                    }
                }
                else
                {
                    if (!SSA::VHDLRealGen::generateCode(outstream, ssa))
                    {
                        doLog(LOG_ERROR, "Error generating VHDL code!\n");
                    }
                }
                closeLogFile();
                return 0; // end program!
            }

            // ------------------------------------------------------------
            // -- INSERT TRUNCATE NODES FOR REG ASSIGNMENTS
            // ------------------------------------------------------------
            if (!SSA::PassRegTrunc::execute(ssa))
            {
                doLog(LOG_ERROR, "REGTRUNC pass failed\n");
            }

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- GENERATE A REFERENCE EVALUATOR TO CHECK OUR PASSES
            // ------------------------------------------------------------

            SSA::Program referenceSSA = ssa;
            SSA::Evaluator eval(referenceSSA);

            eval.randomizeInputValues();
            if (!eval.runProgram())
            {
                printf("Error running reference evaluation program!\n");
                return 1;
            }

            // ------------------------------------------------------------
            // -- CSD PASS
            // ------------------------------------------------------------
            SSA::PassCSDMul::execute(ssa);

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

#if 0
            // ------------------------------------------------------------
            // -- GENERATE AN EVALUATOR TO CHECK THE CSD PASS
            // ------------------------------------------------------------
            SSA::Evaluator eval2(ssa);
            eval2.initInputsFromRefEvaluator(eval);

            if (!eval2.runProgram())
            {
                printf("Error running CSD evaluation program!\n");
                return 1;
            }

            std::stringstream report;
            if (!eval2.compareToRefEvaluator(eval, report))
            {
                doLog(LOG_INFO, "---=========================---\n");
                doLog(LOG_INFO, "---=== EVALUATION FAILED ===---\n");
                doLog(LOG_INFO, "---=========================---\n\n");
            }
            doLog(LOG_INFO, report.str().c_str());
#endif

            // ------------------------------------------------------------
            // -- ADDSUB PASS
            // ------------------------------------------------------------
            if (!SSA::PassAddSub::execute(ssa))
            {
                doLog(LOG_ERROR, "ADDSUB pass failed\n");
            }

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- TRUNCATE PASS
            // ------------------------------------------------------------
            if (!SSA::PassTruncate::execute(ssa))
            {
                doLog(LOG_ERROR, "TRUNCATE pass failed\n");
            }

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- CLEAN PASS
            // ------------------------------------------------------------
            if (!SSA::PassClean::execute(ssa))
            {
                doLog(LOG_ERROR, "Clean pass failed\n");
            }

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- Remove unused variables
            // ------------------------------------------------------------
            if (!SSA::PassRemoveOperands::execute(ssa))
            {
                doLog(LOG_ERROR, "RemoveOperands pass failed\n");
            }

#if 0
            doLog(LOG_INFO, "Variables used:\n");
            for(auto var : ssa.m_operands)
            {
                    doLog(LOG_INFO, "%s %d\n", var->m_identName.c_str(), var.use_count());
            }
#endif

            // ------------------------------------------------------------
            // -- GENERATE AN EVALUATOR TO CHECK THE CSD PASS
            // ------------------------------------------------------------
            SSA::Evaluator eval3(ssa);
            eval3.initInputsFromRefEvaluator(eval);

            doLog(LOG_INFO, "\n\n--== RUNNING VALIDATION ==--\n\n");
            if (!eval3.runProgram())
            {
                printf("Error running final evaluation program!\n");
                return 1;
            }

            std::stringstream report;            
            if (!eval3.compareToRefEvaluator(eval, report))
            {
                doLog(LOG_INFO, "---=========================---\n");
                doLog(LOG_INFO, "---=== EVALUATION FAILED ===---\n");
                doLog(LOG_INFO, "---=========================---\n\n");
                eval3.dumpAllValues(report);
            }
            else
            {
                doLog(LOG_INFO, "---*************************---\n");
                doLog(LOG_INFO, "---*** EVALUATION PASSED ***---\n");
                doLog(LOG_INFO, "---*************************---\n\n");
            }
            doLog(LOG_INFO, report.str().c_str());
            doLog(LOG_INFO, "\n\n\n");


            // ------------------------------------------------------------
            // -- Do more fuzzing
            // ------------------------------------------------------------

            doLog(LOG_INFO, "\n\n--== FUZZING ==--\n\n");
            bool fuzzError = false;
            for(uint32_t i=0; i<1000; i++)
            {
                report.clear();
                eval.randomizeInputValues();
                eval.runProgram();
                eval3.initInputsFromRefEvaluator(eval);
                eval3.runProgram();
                if (!eval3.compareToRefEvaluator(eval, report))
                {
                    fuzzError = true;
                }
            }

            if (fuzzError)
            {
                doLog(LOG_ERROR, "Fuzzing reports errors!\n");
            }
            else
            {
                doLog(LOG_INFO, "Fuzzing tests passed!\n");
            }

            // ------------------------------------------------------------
            // -- VHDL code generation
            // ------------------------------------------------------------
            if (outstream.bad())
            {
                if (!SSA::VHDLCodeGen::generateCode(std::cout, ssa))
                {
                    doLog(LOG_ERROR, "Error generating VHDL code!\n");
                }
            }
            else
            {
                if (!SSA::VHDLCodeGen::generateCode(outstream, ssa, false))
                {
                    doLog(LOG_ERROR, "Error generating VHDL code!\n");
                }
            }
        }
        else
        {
            doLog(LOG_ERROR, "Parse Failed!\n");
            doLog(LOG_ERROR, parse.formatErrors().c_str());
        }
    }

    closeLogFile();

    return 0;
}


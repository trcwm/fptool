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
            // -- GENERATE A REFERENCE EVALUATOR TO CHECK OUR PASSES
            // ------------------------------------------------------------

            SSA::Program referenceSSA = ssa;
            SSA::Evaluator eval(referenceSSA);

            for(auto op : ssa.m_operands)
            {
                const SSA::InputOperand* inOp = dynamic_cast<const SSA::InputOperand*>(op.get());
                if (inOp != NULL)
                {
                    // operand is an input operand; we need to set
                    // a value
                    fplib::SFix *vptr = eval.getValuePtrByName(op->m_identName);
                    if (vptr != NULL)
                    {
                        vptr->randomizeValue();
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "Evaluator could not find input variable :" << op->m_identName;
                        throw std::runtime_error(ss.str());
                    }
                }
            }

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

#if 1
            // ------------------------------------------------------------
            // -- GENERATE AN EVALUATOR TO CHECK THE CSD PASS
            // ------------------------------------------------------------
            SSA::Evaluator eval2(ssa);

            // set inputs to the same value as the reference evaluator
            for(auto op : ssa.m_operands)
            {
                const SSA::InputOperand* inOp = dynamic_cast<const SSA::InputOperand*>(op.get());
                if (inOp != NULL)
                {
                    // operand is an input operand; we need to set
                    // a value
                    fplib::SFix *vptr = eval2.getValuePtrByName(op->m_identName);
                    fplib::SFix *vptr_ref = eval.getValuePtrByName(op->m_identName);
                    if ((vptr != NULL) && (vptr_ref))
                    {
                        vptr->copyValueFrom(vptr_ref);
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "Evaluator could not find input variable :" << op->m_identName;
                        throw std::runtime_error(ss.str());
                    }
                }
            }
            eval2.runProgram();

            // compare the output of the evaluators
            uint32_t opIndex = 0;
            for(auto op : ssa.m_operands)
            {
                SSA::OutputOperand *output = dynamic_cast<SSA::OutputOperand *>(op.get());
                if (output != NULL)
                {
                    fplib::SFix *v1 = eval.getValuePtrByName(op->m_identName);
                    fplib::SFix *v2 = eval2.getValuePtrByName(op->m_identName);
                    if ((v1 == NULL) || (v2 == NULL))
                    {
                        std::stringstream ss;
                        ss << "Evaluator could not find output variable :" << op->m_identName;
                        throw std::runtime_error(ss.str());
                    }
                    if (*v1 != *v2)
                    {
                        doLog(LOG_ERROR, "Evaluation mismatch for output %s\n", op->m_identName.c_str());
                        doLog(LOG_ERROR, "  Reference: Q(%d,%d)\n", v1->intBits(), v1->fracBits());
                        doLog(LOG_ERROR, "  Check    : Q(%d,%d)\n", v2->intBits(), v2->fracBits());
                    }
                    else
                    {
                        doLog(LOG_INFO,"Eval ok for output %s\n", op->m_identName.c_str());
                    }
                }
                opIndex++;
            }
#endif

#if 0
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
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- TRUNCATE PASS
            // ------------------------------------------------------------
            SSA::PassTruncate::execute(ssa);

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- CLEAN PASS
            // ------------------------------------------------------------
            SSA::PassClean::execute(ssa);

            if (verbose)
            {
                std::stringstream ss;
                SSA::Printer::print(ssa, ss, true);
                doLog(LOG_DEBUG, "\n%s", ss.str().c_str());
            }

            // ------------------------------------------------------------
            // -- Remove unused variables
            // ------------------------------------------------------------
            SSA::PassRemoveOperands::execute(ssa);

#if 0
            doLog(LOG_INFO, "Variables used:\n");
            for(auto var : ssa.m_operands)
            {
                    doLog(LOG_INFO, "%s %d\n", var->m_identName.c_str(), var.use_count());
            }
#endif
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
                if (!SSA::VHDLCodeGen::generateCode(outstream, ssa))
                {
                    doLog(LOG_ERROR, "Error generating VHDL code!\n");
                }
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


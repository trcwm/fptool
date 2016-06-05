/*

  FPTOOL - a fixed-point math to VHDL generation tool

*/

#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "reader.h"
#include "tokenizer.h"
#include "parser.h"
#include "ssa.h"
#include "csd.h"
#include "pass_addsub.h"
#include "vhdlcodegen.h"

#define __VERSION__ "0.1a"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("FPTOOL version "__VERSION__" compiled on "__DATE__"\n");
        printf("\nUsage: fptool <source.fp> <result.vhdl>\n\n");
        return 1;
    }
    else
    {
        Reader* reader = Reader::open(argv[1]);
        if (reader == 0)
        {
            printf("Error opening file!\n");
            return 1;
        }

        Tokenizer tokenizer;
        std::vector<token_t> tokens;
        tokenizer.process(reader, tokens);
        delete reader;

        // show the tokens
        uint32_t newlineCnt = 0;
        for(size_t i=0; i<tokens.size(); i++)
        {
            token_t token  = tokens[i];
            if (token.tokID != TOK_NEWLINE)
            {
                newlineCnt = 0;
            }

            switch(token.tokID)
            {
            default:
            case TOK_UNKNOWN:
                printf("Unknown\n");
                break;
            case TOK_NEWLINE:   // suppress superfluous newlines
                if (newlineCnt == 0)
                    printf("\n");
                newlineCnt++;
                break;
            case TOK_IDENT:
                printf("<ident>%s", token.txt.c_str());
                break;
            case TOK_INTEGER:
                printf("<int>%s", token.txt.c_str());
                break;
            case TOK_FLOAT:
                printf("<float>%s", token.txt.c_str());
                break;
            case TOK_PLUS:
                printf(" + ");
                break;
            case TOK_MINUS:
                printf(" - ");
                break;
            case TOK_STAR:
                printf(" * ");
                break;
            case TOK_EQUAL:
                printf(" = ");
                break;
            case TOK_LPAREN:
                printf(" ( ");
                break;
            case TOK_RPAREN:
                printf(" ) ");
                break;
            case TOK_COMMA:
                printf(" , ");
                break;
            case TOK_SEMICOL:
                printf(";");
                break;
            case TOK_SHL:
                printf(" << ");
                break;
            case TOK_SHR:
                printf(" >> ");
                break;
            case TOK_ROL:
                printf(" <<< ");
                break;
            case TOK_ROR:
                printf(" >>> ");
                break;
            case TOK_EOF:
                printf("\nEOF\n");
                break;
            case 100:
                printf("DEFINE");
                break;
            case 101:
                printf("INPUT");
                break;
            case 102:
                printf("CSD");
                break;
            }
        }

        Parser parse;
        statements_t statements;
        if (parse.process(tokens, statements))
        {
            printf("Parse OK!\n");
            /*
            std::cout << std::setprecision(9);
            for(size_t i=0; i<statements.size(); i++)
            {
                printf("statement %d:\n", i);
                statements[i]->dump(std::cout);
            }
            */

            SSACreator ssa;
            SSA::ssaList_t  ssaList;
            SSA::ssaOperands_t ssaOperands;
            if (!ssa.process(statements, ssaList, ssaOperands))
            {
                printf("Error producing SSA: %s\n", ssa.getLastError().c_str());
            }

#if 0
            printf("--== SSA statements ==--\n");
            for(size_t i=0; i<ssaList.size(); i++)
            {
                uint32_t idx1 = ssaList[i].var1;
                uint32_t idx2 = ssaList[i].var2;
                uint32_t idx3 = ssaList[i].var3;
                switch(ssaList[i].operation)
                {
                case SSANode::OP_Add:
                    printf("%s := %s + %s\n", ssaOperandList[idx3].info.txt.c_str(),
                           ssaOperandList[idx2].info.txt.c_str(),
                           ssaOperandList[idx1].info.txt.c_str());
                    break;
                case SSANode::OP_Sub:
                    printf("%s := %s - %s\n", ssaOperandList[idx3].info.txt.c_str(),
                           ssaOperandList[idx2].info.txt.c_str(),
                           ssaOperandList[idx1].info.txt.c_str());
                    break;
                case SSANode::OP_Mul:
                    printf("%s := %s * %s\n", ssaOperandList[idx3].info.txt.c_str(),
                           ssaOperandList[idx2].info.txt.c_str(),
                           ssaOperandList[idx1].info.txt.c_str());
                    break;
                case SSANode::OP_Negate:
                    printf("%s := - %s\n", ssaOperandList[idx3].info.txt.c_str(),
                           ssaOperandList[idx1].info.txt.c_str());
                    break;
                case SSANode::OP_Assign:
                    printf("%s <= %s\n", ssaOperandList[idx3].info.txt.c_str(),
                           ssaOperandList[idx1].info.txt.c_str());
                    break;
                default:
                    printf("** unknown SSA node **\n");
                }
            }
#endif

#if 0
            printf("--== signal information ==--\n");
            for(size_t i=0; i<ssaOperandList.size(); i++)
            {
                switch(ssaOperandList[i].type)
                {
                case operand_t::TypeInput:
                    printf("INPUT: %s Q(%d,%d)\n", ssaOperandList[i].info.txt.c_str(),
                           ssaOperandList[i].info.intBits,
                           ssaOperandList[i].info.fracBits);
                    break;
                case operand_t::TypeOutput:
                    printf("OUTPUT: %s Q(%d,%d)\n", ssaOperandList[i].info.txt.c_str(),
                           ssaOperandList[i].info.intBits,
                           ssaOperandList[i].info.fracBits);
                    break;
                case operand_t::TypeIntermediate:
                    printf("TEMP: %s Q(%d,%d)\n", ssaOperandList[i].info.txt.c_str(),
                           ssaOperandList[i].info.intBits,
                           ssaOperandList[i].info.fracBits);
                    break;
                default:
                    break;
                }
            }
#endif

            PassAddSub  addsub;
            addsub.process(ssaList, ssaOperands);

            VHDLCodeGen codegen;
            codegen.process(ssaList, ssaOperands);
        }
        else
        {
            printf("Parse Failed!\n");
            printf("Line %d pos %d: %s\n", parse.getLastErrorPos().line+1, parse.getLastErrorPos().pos+1, parse.getLastError().c_str());
        }
    }

#if 0
    csd_t d;
    convertToCSD(3.14159265358979, 5, d);
    printf("%f\n", d.value);
    for(uint32_t i=0; i<d.digits.size(); i++)
    {
        if (d.digits[i].sign > 0)
            printf("+2^%d ", d.digits[i].power);
        else
            printf("-2^%d ", d.digits[i].power);
    }
#endif

    return 0;
}


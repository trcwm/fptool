/*

  FPTOOL - a fixed-point math to VHDL generation tool

*/

#include <stdio.h>
#include "reader.h"
#include "tokenizer.h"

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
        for(size_t i=0; i<tokens.size(); i++)
        {
            token_t token  = tokens[i];
            switch(token.tokID)
            {
            default:
            case TOK_UNKNOWN:
                printf("Unknown\n");
                break;
            case TOK_NEWLINE:
                printf("\n");
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
    }

    return 0;
}


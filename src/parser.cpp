/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  The parser takes a list of tokens,
                checks the grammar and produces
                a parse tree.

  Author: Niels A. Moseley

*/

#include <iostream>
#include "parser.h"


Parser::Parser() : m_tokens(NULL)
{
}

void Parser::error(const std::string &txt)
{
    std::cout << txt.c_str() << std::endl;
}

ParseTreeNode* Parser::process(const std::vector<token_t> &tokens)
{
    if (tokens.size() == 0)
    {
        m_lastError = std::string("Error: token list is empty");
        return NULL;
    }

    // prepare for iteration
    m_tokens = &tokens;

    state_t state;
    state.node = 0;
    state.tokIdx = 0;

    if (!acceptProgram(state))
    {
        return NULL;
    }

    return NULL;
}

bool Parser::acceptProgram(state_t &s)
{
    // check for a list of statements,
    // ending with EOF
    if (!acceptStatementlist(s))
    {
        return false;
    }

    token_t tok = getToken(s);
    if (tok.tokID != TOK_EOF)
    {
        error("End of file expected");
        return false;
    }

    return true;
}

bool Parser::acceptStatementlist(state_t &s)
{
    state_t savestate = s;
    if (acceptStatement(s) && acceptStatementlist(s))
    {
        return true;
    }

    s = savestate;
    // empty clause
    return true;
}

bool Parser::acceptStatement(state_t &s)
{
    state_t savestate = s;
    if (acceptDefinition(s))
    {
        token_t tok = getToken(s);
        if (tok.tokID == TOK_SEMICOL)
        {
            next(s);
            return true;
        }
        s = savestate;
        error("Expected ';'");
    }
    else if (acceptAssignment(s))
    {
        token_t tok = getToken(s);
        if (tok.tokID == TOK_SEMICOL)
        {
            next(s);
            return true;
        }
        s = savestate;
        error("Expected ';'");
    }
    else
    {
        token_t tok = getToken(s);
        if (tok.tokID == TOK_NEWLINE)
        {
            next(s);
            return true;
        }
    }
    s = savestate;
    return false;
}

bool Parser::acceptDefinition(state_t &s)
{
    state_t savestate = s;
    token_t tok = getToken(s);
    if (tok.tokID != TOK_DEFINE)
    {
        s = savestate;
        return false;
    }
    tok = next(s);
    if (tok.tokID != TOK_IDENT)
    {
        error("Identifier expected after DEFINE");
        s = savestate;
        return false;
    }
    //TODO: store IDENT
    tok = next(s);
    if (tok.tokID != TOK_EQUAL)
    {
        error("'=' expected");
        s = savestate;
        return false;
    }
    tok = next(s);
    if (!acceptDefspec(s))
    {
        s = savestate;
        return false;
    }
    return true;
}

bool Parser::acceptDefspec(state_t &s)
{
    state_t savestate = s;
    token_t tok = getToken(s);

    // first check 'INPUT'
    if (tok.tokID == TOK_INPUT)
    {
        tok = next(s);
        if (tok.tokID != TOK_LPAREN)
        {
            error("Expected '('");
            s = savestate;
            return false;
        }
        tok = next(s);
        if (tok.tokID != TOK_INTEGER)
        {
            error("Expected an INTEGER");
            s = savestate;
            return false;
        }
        tok = next(s);
        if (tok.tokID != TOK_COMMA)
        {
            error("Expected ','");
            s = savestate;
            return false;
        }
        tok = next(s);
        if (tok.tokID != TOK_INTEGER)
        {
            error("Expected an INTEGER");
            s = savestate;
            return false;
        }
        tok = next(s);
        if (tok.tokID != TOK_RPAREN)
        {
            error("Expected ')'");
            s = savestate;
            return false;
        }
        next(s);
    }
    // check CSD
    else if (tok.tokID == TOK_CSD)
    {
        tok = next(s);
        if (tok.tokID != TOK_LPAREN)
        {
            error("Expected '('");
            s = savestate;
            return false;
        }
        tok = next(s);
        if (tok.tokID != TOK_FLOAT)
        {
            error("Expected a FLOAT");
            s = savestate;
            return false;
        }
        tok = next(s);
        if (tok.tokID != TOK_COMMA)
        {
            error("Expected ','");
            s = savestate;
            return false;
        }
        tok = next(s);
        if (tok.tokID != TOK_INTEGER)
        {
            error("Expected an INTEGER");
            s = savestate;
            return false;
        }
        tok = next(s);
        if (tok.tokID != TOK_RPAREN)
        {
            error("Expected ')'");
            s = savestate;
            return false;
        }
        next(s);
    }
    else
    {
        error("Expected INPUT or CSD");
        s = savestate;
        return false;
    }
    return true;
}

bool Parser::acceptAssignment(state_t &s)
{
    state_t savestate = s;
    token_t tok = getToken(s);
    if (tok.tokID != TOK_IDENT)
    {
        s = savestate;
        return false;
    }
    tok = next(s);
    if (tok.tokID != TOK_EQUAL)
    {
        error("Expected '='");
        s = savestate;
        return false;
    }
    tok = next(s);
    if (!acceptExpr(s))
    {
        error("Expression expected");
        s = savestate;
        return false;
    }
    return true;
}

bool Parser::acceptExpr(state_t &s)
{
    state_t savestate = s;
    token_t tok = getToken(s);

    // TRY: term + expr
    if (!acceptTerm(s))
    {
        s = savestate;
        return false;
    }

    // term accepted, so we must now check:
    //   + expr
    //   - expr
    //   term

    tok = getToken(s);
    if (tok.tokID == TOK_PLUS)
    {
        tok = next(s);
        if (!acceptExpr(s))
        {
            //TODO: check if we really should be returning to this
            // state or whether another (later) state is more appropriate.
            s = savestate;
            error("Expected an expression");
            return false;
        }
        return true;    // TERM + EXPR
    }
    else if (tok.tokID == TOK_MINUS)
    {
        tok = next(s);
        if (!acceptExpr(s))
        {
            //TODO: check if we really should be returning to this
            // state or whether another (later) state is more appropriate.
            s = savestate;
            error("Expected an expression");
            return false;
        }
        return true;    // TERM - EXPR
    }

    // if we end up here, we've already accpted a single term!
    // and that's ok!

    return true;
}

bool Parser::acceptTerm(state_t &s)
{
    state_t savestate = s;
    token_t tok = getToken(s);

    // try: '-' factor
    if (tok.tokID == TOK_MINUS)
    {
        tok = next(s);
        if (!acceptFactor(s))
        {
            error("Expected a factor");
            s = savestate;
            return false;
        }
    }
    else if (acceptFactor(s))
    {
        // factor * term
        // or just factor.

        tok = getToken(s);
        if (tok.tokID == TOK_STAR)
        {
            state_t savestate2 = s;
            tok = next(s);
            if (acceptTerm(s))
            {
                return true;    // accepted: factor * term
            }
            error("Expected a term");
            s = savestate2;
            return false;
        }

        return true; // just a factor
    }
    s = savestate;
    return false;
}

bool Parser::acceptFactor(state_t &s)
{
    state_t savestate = s;
    token_t tok = getToken(s);

    if (tok.tokID == TOK_INTEGER)
    {
        next(s);
        return true;    // INTEGER
    }

    if (tok.tokID == TOK_FLOAT)
    {
         next(s);
         return true;   // FLOAT
    }

    if (tok.tokID == TOK_IDENT)
    {
        next(s);
        return true;    // IDENTIFIER
    }

    if (tok.tokID == TOK_LPAREN)
    {
        tok = next(s);
        if (!acceptExpr(s))
        {
            error("Expected an expression");
            s = savestate;
            return false;
        }
        tok = getToken(s);
        if (tok.tokID != TOK_RPAREN)
        {
            error("Expected ')'");
            s = savestate;
            return false;
        }
        next(s);
        return true;    // ( expr )
    }

    s = savestate;
    return false;
}

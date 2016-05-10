/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  The parser takes a list of tokens,
                checks the grammar and produces
                a parse tree.

  Author: Niels A. Moseley

*/

#ifndef parser_h
#define parser_h

#include <string>
#include <vector>
#include "tokenizer.h"

/** Parse tree node type structure */
struct ParseTreeNode
{
    ParseTreeNode()
    {
        left = 0;
        right = 0;
        type = NodeUnkown;
    }

    enum node_t {NodeUnkown, NodeAdd, NodeSub, NodeMul,
                 NodeAssign, NodeDefine,
                 NodeIdent,
                 NodeInteger, NodeFloat
                };

    node_t        type;
    std::string   txt;      // identifier name, integer or float value.
    ParseTreeNode *left;
    ParseTreeNode *right;
};

/** Parse tree generator object */
class Parser
{
public:
    Parser();

    /** Process a list of tokens and produce a parse tree.
        NULL is returned when a parse error occurs. */
    ParseTreeNode* process(const std::vector<token_t> &tokens);

    std::string getLastError() const
    {
        return m_lastError;
    }


protected:
    struct state_t
    {
        size_t        tokIdx;
        ParseTreeNode *node;
    };

    /* The following methods return true if the tokens starting from
       index 'tokIdx' are consistent with the production from the
       FPTOOL grammar.
    */
    bool acceptProgram(state_t &s);
    bool acceptStatementlist(state_t &s);
    bool acceptStatement(state_t &s);
    bool acceptDefinition(state_t &s);
    bool acceptDefspec(state_t &s);
    bool acceptAssignment(state_t &s);
    bool acceptExpr(state_t &s);
    bool acceptTerm(state_t &s);
    bool acceptFactor(state_t &s);

    /** Advance the token index and get the next token */
    token_t next(state_t &s)
    {
        s.tokIdx++;
        return getToken(s);
    }

    /** Get the current token */
    token_t getToken(const state_t &s)
    {
        token_t dummy_token;

        if (m_tokens == 0)
            return dummy_token;

        if (s.tokIdx < m_tokens->size())
        {
            return m_tokens->at(s.tokIdx);
        }
        else
        {
            return dummy_token;
        }
    }

    /** Report an error */
    void error(const std::string &txt);

    std::string   m_lastError;
    const std::vector<token_t>  *m_tokens;
};

#endif

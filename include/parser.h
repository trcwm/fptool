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
#include <memory>

#include "tokenizer.h"
#include "astnode.h"

typedef std::shared_ptr<ASTNode> ASTNodePtr;

typedef std::vector<ASTNode*> statements_t;

/** Parser to translate token stream from tokenizer/lexer to operation stack. */
class Parser
{
public:
    Parser();

    /** Process a list of tokens and list of statements.
        false is returned when a parse error occurs.
        When an error occurs, call getLastError() to get
        a human-readable string of the error.
    */
    bool process(const std::vector<token_t> &tokens, statements_t &result);

    /** Return a description of the last parse error that occurred. */
    std::string getLastError() const
    {
        return m_lastError;
    }

    /** Get the position in the source code where the last error occurred. */
    Reader::position_info getLastErrorPos() const
    {
        return m_lastErrorPos;
    }

protected:
    struct state_t
    {
        size_t        tokIdx;
        Reader::position_info tokPos;
    };

    /* The following methods return true if the tokens starting from
       index 'tokIdx' are consistent with the production from the
       FPTOOL grammar.

       All functions return false when the production was not succesful.
       Each method is responsible for filling in the 'newNode' information
       and creating any subnodes needed for further processing by
       recusively calling other accpet methods.
    */

    bool acceptProgram(state_t &s, statements_t &result);
    ASTNode* acceptDefinition(state_t &s);

    ASTNode *acceptDefspec(state_t &s);
    ASTNode *acceptDefspec1(state_t &s);
    ASTNode *acceptDefspec2(state_t &s);

    // functions
    ASTNode* acceptTruncate(state_t &s);

    ASTNode *acceptAssignment(state_t &s);

    ASTNode* acceptExpr(state_t &s);
    ASTNode* acceptExprAccent(state_t &s, ASTNode *leftNode);
    ASTNode* acceptExprAccent1(state_t &s, ASTNode *leftNode);
    ASTNode* acceptExprAccent2(state_t &s, ASTNode *leftNode);

    ASTNode* acceptTerm(state_t &s);
    ASTNode* acceptTermAccent(state_t &s, ASTNode *leftNode);
    ASTNode* acceptTermAccent1(state_t &s, ASTNode *leftNode);
    ASTNode* acceptTermAccent2(state_t &s, ASTNode *leftNode);

    ASTNode *acceptFactor(state_t &s);
    ASTNode* acceptFactor1(state_t &s);
    ASTNode* acceptFactor2(state_t &s);
    ASTNode* acceptFactor3(state_t &s);



    /** match a token, return true if matched and advance the token index. */
    bool match(state_t &s, uint32_t tokenID);

    /** match a NULL-terminated list of tokens. */
    bool matchList(state_t &s, const uint32_t *tokenIDlist);

    /** Advance the token index and get the next token */
    token_t next(state_t &s)
    {
        s.tokIdx++;
        token_t tok = getToken(s);
        s.tokPos = tok.pos;
        return getToken(s);
    }

    /** Get the current token, which or without an offset w.r.t.*/
    token_t getToken(const state_t &s, int32_t offset = 0)
    {
        token_t dummy_token;

        dummy_token.tokID = TOK_UNKNOWN;

        if (m_tokens == 0)
            return dummy_token;

        if ((s.tokIdx+offset) < m_tokens->size())
        {
            return m_tokens->at(s.tokIdx+offset);
        }
        else
        {
            return dummy_token;
        }
    }

    /** Report an error */
    void error(const state_t &s, const std::string &txt);
    void error(uint32_t dummy, const std::string &txt);

    std::string   m_lastError;
    Reader::position_info m_lastErrorPos;
    const std::vector<token_t>  *m_tokens;
};

#endif

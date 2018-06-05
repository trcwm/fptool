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
#include "identdb.h"

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
    bool process(const std::vector<token_t> &tokens, AST::Statements &statements, SymbolTable &symbols);

    /** check if the parser produced any errors */
    bool hasErrors() const
    {
        return !m_errors.empty();
    }

    /** return all errors as one big formatted string */
    std::string formatErrors() const;

protected:
    void clearErrors()
    {
        for(auto error: m_errors)
        {
            delete error;
        }
        m_errors.clear();
    }


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

    bool acceptProgram(state_t &s, AST::Statements &result);
    AST::ASTNodeBase* acceptDefinition(state_t &s);

    AST::Declaration *acceptDefspec(state_t &s, const std::string &identifier);
    AST::InputDeclaration *acceptDefspec1(state_t &s);      ///< accept an input declaration
    AST::RegDeclaration   *acceptDefspec3(state_t &s);      ///< accept a register declaration
    AST::CSDDeclaration   *acceptDefspec2(state_t &s);      ///< accept a CSD declaration

    // functions
    AST::ASTNodeBase* acceptTruncate(state_t &s);

    AST::ASTNodeBase* acceptAssignment(state_t &s);

    AST::ASTNodeBase* acceptExpr(state_t &s);
    AST::ASTNodeBase* acceptExprAccent(state_t &s, AST::ASTNodeBase *leftNode);
    AST::ASTNodeBase* acceptExprAccent1(state_t &s, AST::ASTNodeBase *leftNode);
    AST::ASTNodeBase* acceptExprAccent2(state_t &s, AST::ASTNodeBase *leftNode);

    AST::ASTNodeBase* acceptTerm(state_t &s);
    AST::ASTNodeBase* acceptTermAccent(state_t &s, AST::ASTNodeBase *leftNode);
    AST::ASTNodeBase* acceptTermAccent1(state_t &s, AST::ASTNodeBase *leftNode);
    AST::ASTNodeBase* acceptTermAccent2(state_t &s, AST::ASTNodeBase *leftNode);

    AST::ASTNodeBase* acceptFactor(state_t &s);
    AST::ASTNodeBase* acceptFactor1(state_t &s);
    AST::ASTNodeBase* acceptFactor2(state_t &s);
    AST::ASTNodeBase* acceptFactor3(state_t &s);

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

    struct error_t
    {
        std::string             m_errstr;   ///< human readable error string
        Reader::position_info   m_pos;      ///< error position in the source
    };

    std::list<error_t*>         m_errors;   ///< list of errors

    SymbolTable                 *m_symTable; ///< the symbol table
    const std::vector<token_t>  *m_tokens;
};

#endif

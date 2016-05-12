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

/** Abstract Syntax Tree Node */
struct ASTNode
{
  enum node_t {NodeUnknown, NodeHead,
               NodeStatement,
               NodeAssign, NodeDefine,
               NodeInput, NodeCSD,
               NodeAdd, NodeSub, NodeMul,
               NodeUnaryMinus,
               NodeIdent,
               NodeInteger, NodeFloat,
              };

    ASTNode(node_t nodeType = NodeUnknown)
    {
        type = nodeType;
    }

    node_t          type;
    std::string     txt;      // identifier name, integer or float value.
    int32_t         fracBits;
    int32_t         intBits;
    int32_t         csdBits;
    double          csdFloat;
    int32_t         intVal;

    std::shared_ptr<ASTNode>  left;
    std::shared_ptr<ASTNode>  right;
};

typedef std::shared_ptr<ASTNode> ASTNodePtr;

typedef std::vector<ASTNodePtr> statements_t;

/** Parser to translate token stream from tokenizer/lexer to operation stack. */
class Parser
{
public:
    Parser();

    /** Process a list of tokens and produce a operations stack.
        false is returned when a parse error occurs. */
    bool process(const std::vector<token_t> &tokens, statements_t &result);

    std::string getLastError() const
    {
        return m_lastError;
    }

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
       recusively calling other methods.
    */

    bool acceptProgram(state_t &s, statements_t &result);
    bool acceptDefinition(state_t &s, ASTNodePtr newNode);

    bool acceptDefspec(state_t &s, ASTNodePtr newNode);
    bool acceptDefspec1(state_t &s, ASTNodePtr newNode);
    bool acceptDefspec2(state_t &s, ASTNodePtr newNode);

    bool acceptAssignment(state_t &s, ASTNodePtr newNode);
    bool acceptExpr(state_t &s, ASTNodePtr newNode);
    bool acceptExpr1(state_t &s, ASTNodePtr newNode);
    bool acceptExpr2(state_t &s, ASTNodePtr newNode);

    bool acceptTerm(state_t &s, ASTNodePtr newNode);
    bool acceptTerm1(state_t &s, ASTNodePtr newNode);
    bool acceptTerm2(state_t &s, ASTNodePtr newNode);

    bool acceptFactor(state_t &s, ASTNodePtr newNode);
    bool acceptFactor1(state_t &s, ASTNodePtr newNode);

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

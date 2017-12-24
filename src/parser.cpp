/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  The parser takes a list of tokens,
                checks the grammar and produces
                a parse tree.

  Author: Niels A. Moseley

*/

#include <iostream>
#include "parser.h"
#include "csd.h"

Parser::Parser() : m_tokens(NULL)
{
}

void Parser::error(const state_t &s, const std::string &txt)
{
    m_lastError = txt;
    m_lastErrorPos = s.tokPos;
    std::cout << txt.c_str() << std::endl;
}

void Parser::error(uint32_t dummy, const std::string &txt)
{
    m_lastError = txt;
    std::cout << txt.c_str() << std::endl;
}

bool Parser::match(state_t &s, uint32_t tokenID)
{
    token_t tok = getToken(s);
    if (tok.tokID != tokenID)
    {
        return false;
    }
    next(s);
    return true;
}

bool Parser::matchList(state_t &s, const uint32_t *tokenIDlist)
{
    while (*tokenIDlist != NULL)
    {
        if (!match(s, *tokenIDlist++))
            return false;
    }
    return true;
}

bool Parser::process(const std::vector<token_t> &tokens, statements_t &result)
{
    m_lastError.clear();

    if (tokens.size() == 0)
    {
        error(0,"Internal error: token list is empty");
        return false;
    }

    // prepare for iteration
    m_tokens = &tokens;

    state_t state;
    state.tokIdx = 0;

    return acceptProgram(state, result);
}

bool Parser::acceptProgram(state_t &s, statements_t &statements)
{
    // productions: definition | assignment | NEWLINE | EOF

    bool productionAccepted = true;
    token_t tok = getToken(s);


    while(productionAccepted == true)
    {
        productionAccepted = false;
        
        ASTNode *node = 0;
        if ((node=acceptDefinition(s)) != 0)
        {
            productionAccepted = true;
            statements.push_back(node);
        }
        else if ((node=acceptAssignment(s)) != 0)
        {
            productionAccepted = true;
            statements.push_back(node);
        }
        else if (match(s, TOK_NEWLINE))
        {
            productionAccepted = true;
            // do nothing..
        }
        else if (match(s, TOK_EOF))
        {
            return true;
        }
    }
    return false;
}


ASTNode* Parser::acceptDefinition(state_t &s)
{
    // production: DEFINE IDENT EQUAL declspec SEMICOL

    state_t savestate = s;
    if (!match(s, TOK_DEFINE))
    {
        s = savestate;
        return false;
    }

    if (!match(s, TOK_IDENT))
    {
        error(s,"Identifier expected after DEFINE");
        s = savestate;
        return false;
    }

    if (!match(s, TOK_EQUAL))
    {
        error(s,"'=' expected");
        s = savestate;
        return false;
    }

    std::string identifier = getToken(s, -2).txt;    // get identifier string

    // create new RHS specification node:
    ASTNode *declNode = 0;
    if ((declNode=acceptDefspec(s)) == 0)
    {
        error(s,"Expected a declaration.");
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_SEMICOL))
    {
        error(s,"Definitions should end with a semicolon.");
        s = savestate;
        delete declNode;
        return NULL;
    }

    //FIXME: is this correct!?!
    declNode->info.txt = identifier;

    return declNode;
}

ASTNode* Parser::acceptDefspec(state_t &s)
{
    // productions: defspec1 | defspec2

    ASTNode *node = 0;
    if ((node=acceptDefspec1(s)) != NULL)
    {
        return node;
    }
    else if ((node=acceptDefspec2(s)) != NULL)
    {
        return node;
    }
    return NULL;
}

ASTNode* Parser::acceptDefspec1(state_t &s)
{
    // production: INPUT LPAREN INTEGER COMMA INTEGER RPAREN

    const uint32_t tokenList[] =
        {TOK_INPUT, TOK_LPAREN, TOK_INTEGER, TOK_COMMA, TOK_INTEGER, TOK_RPAREN, 0};

    state_t savestate = s;
    if (!matchList(s, tokenList))
    {
        s=savestate;
        return false;
    }

    ASTNode* newNode = new ASTNode();
    newNode->type = ASTNode::NodeInput;
    newNode->info.intBits = atoi(getToken(s, -4).txt.c_str()); // first integer
    newNode->info.fracBits = atoi(getToken(s, -2).txt.c_str()); // second integer

    return newNode;
}


ASTNode* Parser::acceptDefspec2(state_t &s)
{
    // production: CSD LPAREN FLOAT COMMA INTEGER RPAREN

    const uint32_t tokenList[] =
        {TOK_CSD, TOK_LPAREN, TOK_FLOAT, TOK_COMMA, TOK_INTEGER, TOK_RPAREN, 0};

    state_t savestate = s;
    if (!matchList(s, tokenList))
    {
        s=savestate;
        return NULL;
    }

    ASTNode *newNode = new ASTNode();
    newNode->type = ASTNode::NodeCSD;
    newNode->info.csdFloat = atof(getToken(s, -4).txt.c_str()); // first argument
    newNode->info.csdBits = atoi(getToken(s, -2).txt.c_str()); // second argument

    csd_t csd;
    if (!convertToCSD(newNode->info.csdFloat, newNode->info.csdBits, csd))
    {
        error(s,"acceptDefspec2: cannot convert CSD");
    }

    float msb = ceil(log10(fabs(static_cast<double>(csd.value)))/log10(2.0));
    newNode->info.intBits = static_cast<int32_t>(msb+1); // add sign bit
    newNode->info.fracBits = -csd.digits.back().power;
    newNode->info.csd = csd;
    return newNode;
}


ASTNode* Parser::acceptTruncate(state_t &s)
{
    // production: TRUNCATE LPAREN EXPR COMMA INTEGER COMMA INTEGER RPAREN
    state_t savestate = s;

    if (!match(s, TOK_TRUNC))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_LPAREN))
    {
        error(s,"Left parenthesis expected");
        s = savestate;
        return NULL;
    }

    ASTNode* exprNode = Parser::acceptExpr(s);
    if (!exprNode)
    {
        error(s,"Expression expected");
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_COMMA))
    {
        delete exprNode;
        error(s,"Comma expected");
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_INTEGER))
    {
        delete exprNode;
        error(s,"Integer expected");
        s = savestate;
        return NULL;
    }

    int32_t intbits = atoi(getToken(s, -1).txt.c_str());

    if (!match(s, TOK_COMMA))
    {
        delete exprNode;
        error(s,"Comma expected");
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_INTEGER))
    {
        delete exprNode;
        error(s,"Integer expected");
        s = savestate;
        return NULL;
    }

    int32_t fracbits = atoi(getToken(s, -1).txt.c_str());

    if (!match(s, TOK_RPAREN))
    {
        error(s,"Right parenthesis expected");
        s = savestate;
        return NULL;
    }

    ASTNode *newNode = new ASTNode();
    newNode->type = ASTNode::NodeTruncate;
    newNode->info.intBits  = intbits;
    newNode->info.fracBits = fracbits;
    newNode->right = exprNode;

    return newNode;
}

ASTNode* Parser::acceptAssignment(state_t &s)
{
    // production: IDENT EQUAL expr SEMICOL
    state_t savestate = s;

    if (!match(s,TOK_IDENT))
    {
        s = savestate;
        return false;
    }

    if (!match(s,TOK_EQUAL))
    {
        error(s,"Expected '='");
        s = savestate;
        return false;
    }

    std::string identifier = getToken(s, -2).txt;

    ASTNode *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == 0)
    {
        error(s,"Expression expected");
        s = savestate;
        return false;
    }

    if (!match(s, TOK_SEMICOL))
    {
        error(s,"Assignments must end with a semicolon.");
        s = savestate;
        delete exprNode;
        return NULL;
    }

    ASTNode *newNode = new ASTNode();
    newNode->type = ASTNode::NodeAssign;
    newNode->info.txt  = identifier;
    newNode->right = exprNode;

    return newNode;
}

ASTNode* Parser::acceptExpr(state_t &s)
{
    // productions: term expr'
    //
    // the term is the first term
    // and must therefore be
    // added as the left leaf
    //

    state_t savestate = s;

    ASTNode *leftNode = 0;
    if ((leftNode=acceptTerm(s)) != NULL)
    {
        // the term is the left-hand size of the expr'
        // the right hand side and the operation node
        // itself still need to be found.
        //
        // note, exprAccentNode is never NULL
        // because of it's epsilon solution
        //
        ASTNode *exprAccentNode = acceptExprAccent(s, leftNode);
        return exprAccentNode;
    }
    s = savestate;
    return NULL;
}

ASTNode* Parser::acceptExprAccent(state_t &s, ASTNode *leftNode)
{
    // production: - term expr' | + term expr' | epsilon
    //
    // we already have the left-hand side of the
    // addition or subtraction.
    //
    // if we encounter the epsilon,
    // the resulting node is just
    // the leftNode, which was already
    // matched
    //

    state_t savestate = s;

    ASTNode *topNode = 0;
    if ((topNode = acceptExprAccent1(s, leftNode)) != 0)
    {
        return topNode;
    }

    s = savestate;
    if ((topNode = acceptExprAccent2(s, leftNode)) != 0)
    {
        return topNode;
    }

    // if nothing matched, that's ok
    // because we have an epsilon
    // solution
    s = savestate;
    return leftNode;
}

ASTNode* Parser::acceptExprAccent1(state_t &s, ASTNode *leftNode)
{
    // production: - term expr'
    state_t savestate = s;

    if (!match(s, TOK_MINUS))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptExprAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeSub);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptExprAccent2(state_t &s, ASTNode *leftNode)
{
    // production: + term expr'
    state_t savestate = s;

    if (!match(s, TOK_PLUS))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptTerm(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptExprAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeAdd);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptExprAccent will never return NULL
    ASTNode *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}


ASTNode* Parser::acceptTerm(state_t &s)
{
    // production: factor term'
    state_t savestate = s;

    ASTNode *leftNode = 0;
    if ((leftNode=acceptFactor(s)) != NULL)
    {
        // the term is the left-hand size of the term'
        // the right hand side and the operation node
        // itself still need to be found.
        //
        // note, termAccentNode is never NULL
        // because of it's epsilon solution
        //
        ASTNode *termAccentNode = acceptTermAccent(s, leftNode);
        return termAccentNode;
    }

    s = savestate;
    return NULL;
}

ASTNode* Parser::acceptTermAccent(state_t &s, ASTNode *leftNode)
{
    // production: * factor term' | / factor term' | epsilon
    //
    // we already have the left-hand side of the
    // multiplication or division.
    //
    // if we encounter the epsilon,
    // the resulting node is just
    // the leftNode, which was already
    // matched
    //

    state_t savestate = s;

    ASTNode *topNode = 0;
    if ((topNode=acceptTermAccent1(s, leftNode)) != NULL)
    {
        return topNode;
    }

    s = savestate;
    if ((topNode = acceptTermAccent2(s, leftNode)) != NULL)
    {
        return topNode;
    }

    // if nothing matched, that's ok
    // because we have an epsilon
    // solution
    s = savestate;
    return leftNode;
}

ASTNode* Parser::acceptTermAccent1(state_t &s, ASTNode *leftNode)
{
    // production: * factor term'
    state_t savestate = s;

    if (!match(s, TOK_STAR))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptFactor(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptTermAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeMul);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptTermAccent will never return NULL
    ASTNode *headNode = acceptTermAccent(s, operationNode);
    return headNode;
}

ASTNode* Parser::acceptTermAccent2(state_t &s, ASTNode *leftNode)
{
    // production: / factor term'
    state_t savestate = s;

    if (!match(s, TOK_SLASH))
    {
        return NULL;
    }

    ASTNode *rightNode = 0;
    if ((rightNode=acceptFactor(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // create a new 'head' node here
    // with the right leaf being
    // the term that was just found
    // and the left leaf the argument node
    //
    // supply the new head node to the next
    // acceptTermAccent function

    ASTNode *operationNode = new ASTNode(ASTNode::NodeDiv);
    operationNode->left = leftNode;
    operationNode->right = rightNode;

    // note: acceptTermAccent will never return NULL
    ASTNode *headNode = acceptTermAccent(s, operationNode);
    return headNode;
}


ASTNode* Parser::acceptFactor(state_t &s)
{
    state_t savestate = s;

    // FUNCTION ( expr )
    ASTNode *factorNode = 0;

    if ((factorNode=acceptFactor1(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // ( expr )
    if ((factorNode=acceptFactor2(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // - factor
    if ((factorNode=acceptFactor3(s)) != NULL)
    {
        return factorNode;
    }

    s = savestate;
    // INTEGER
    if (match(s, TOK_INTEGER))
    {
        factorNode = new ASTNode(ASTNode::NodeInteger);
        factorNode->info.intVal = atoi(getToken(s, -1).txt.c_str());
        // calculate the int and frac bits for the integer
        factorNode->info.intBits = static_cast<int32_t>(pow(2.0f,ceil(log10((float)factorNode->info.intVal)/log10(2.0f))))+1;
        factorNode->info.fracBits = 0;
        return factorNode;
    }

    // FLOAT
    if (match(s, TOK_FLOAT))
    {
        error(s, "literal floats are not supported!");
        return NULL;
        //newNode->type = ASTNode::NodeIdent;
        //newNode->info.txt = getToken(s, -1).txt;
        //return true;    // IDENT
    }

    // IDENTIFIER
    if (match(s, TOK_IDENT))
    {
        factorNode = new ASTNode(ASTNode::NodeIdent);
        factorNode->info.txt = getToken(s, -1).txt;
        return factorNode;    // IDENT
    }

    error(s, "Factor is not an integer, float, identifier or parenthesised expression.");
    return false;
}

ASTNode* Parser::acceptFactor1(state_t &s)
{    
    // production: FUNCTION ( expr )

    ASTNode *node = NULL;
    if ((node=acceptTruncate(s)) != NULL)
    {
        return node;
    }

    return node;

#if 0
    state_t savestate = s;
    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return false;
    }
    if (!acceptExpr(s, newNode))
    {
        s = savestate;
        return false;
    }
    if (!match(s, TOK_RPAREN))
    {
        s = savestate;
        return false;
    }
    return true;
#endif
}

ASTNode* Parser::acceptFactor2(state_t &s)
{
    // ( expr )

    state_t savestate = s;
    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return NULL;
    }
    ASTNode *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }
    if (!match(s, TOK_RPAREN))
    {
        delete exprNode;
        s = savestate;
        return NULL;
    }
    return exprNode;
}

ASTNode* Parser::acceptFactor3(state_t &s)
{
    // production: - factor
    state_t savestate = s;
    if (!match(s, TOK_MINUS))
    {
        s = savestate;
        return NULL;
    }

    ASTNode *factorNode = 0;
    if ((factorNode=acceptFactor(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // unary minus node
    ASTNode *exprNode = new ASTNode(ASTNode::NodeUnaryMinus);
    exprNode->left = 0;
    exprNode->right = factorNode;

    return exprNode;
}

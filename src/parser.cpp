/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  The parser takes a list of tokens,
                checks the grammar and produces
                a parse tree.

  Author: Niels A. Moseley

*/

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "parser.h"
#include "csd.h"

Parser::Parser() : m_tokens(NULL)
{
    m_errors.clear();
}

void Parser::error(const state_t &s, const std::string &txt)
{
    error_t *description = new error_t;
    description->m_errstr = txt;
    description->m_pos    = s.tokPos;
    m_errors.push_back(description);
}

void Parser::error(uint32_t dummy, const std::string &txt)
{
    error_t *description = new error_t;
    description->m_errstr = txt;
    description->m_pos.line = 0;
    description->m_pos.offset = 0;
    description->m_pos.pos = 0;
    m_errors.push_back(description);
}

std::string Parser::formatErrors() const
{
    std::stringstream ss;
    for(auto error : m_errors)
    {
        ss << "Line " << error->m_pos.line << " offset " << error->m_pos.offset << " ";
        ss << error->m_errstr << "\n";
    }
    return ss.str();
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
    while (*tokenIDlist != 0)
    {
        if (!match(s, *tokenIDlist++))
            return false;
    }
    return true;
}

bool Parser::process(const std::vector<token_t> &tokens,
                     AST::Statements &result,
                     SymbolTable &symbols)
{
    // clear the database in case it wasn't empty.
    symbols.clear();

    m_identDB = &symbols;

    bool ok = false;
    try
    {
        if (tokens.size() == 0)
        {
            error(0,"Internal error: token list is empty");
            return false;
        }

        // prepare for iteration
        m_tokens = &tokens;

        state_t state;
        state.tokIdx = 0;
        ok = acceptProgram(state, result);
    }
    catch(std::runtime_error &e)
    {
        return false;
    }

    return ok;
}

bool Parser::acceptProgram(state_t &s, AST::Statements &statements)
{
    // productions: definition | assignment | NEWLINE | EOF

    bool productionAccepted = true;
    //token_t tok = getToken(s);

    while(productionAccepted == true)
    {
        productionAccepted = false;
        
        AST::ASTNodeBase *node = 0;
        if ((node=acceptDefinition(s)) != 0)
        {
            productionAccepted = true;
            statements.m_statements.push_back(node);
        }
        else if ((node=acceptAssignment(s)) != 0)
        {
            productionAccepted = true;
            statements.m_statements.push_back(node);
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


AST::ASTNodeBase* Parser::acceptDefinition(state_t &s)
{
    // production: DEFINE IDENT EQUAL declspec SEMICOL

    state_t savestate = s;
    if (!match(s, TOK_DEFINE))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_IDENT))
    {
        error(s,"Identifier expected after DEFINE");
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_EQUAL))
    {
        error(s,"'=' expected");
        s = savestate;
        return NULL;
    }

    std::string identifier = getToken(s, -2).txt;    // get identifier string

    AST::Declaration *declNode = 0;
    if ((declNode=acceptDefspec(s, identifier)) == 0)
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

    declNode->m_identName = identifier;

    return declNode;
}

AST::Declaration* Parser::acceptDefspec(state_t &s, const std::string &identifier)
{
    // productions: defspec1 | defspec2 | defspec3

    AST::Declaration *node = 0;
    if ((node=acceptDefspec1(s)) != NULL)
    {
        // INPUT node
        if (!m_identDB->addIdentifier(identifier, SymbolTable::info_t::T_INPUT))
        {
            error(s, "Identifier already exists!");
            return NULL;
        }
        return node;
    }
    else if ((node=acceptDefspec2(s)) != NULL)
    {
        // CSD node
        if (!m_identDB->addIdentifier(identifier, SymbolTable::info_t::T_CSD))
        {
            error(s, "Identifier already exists!");
            return NULL;
        }
        return node;
    }
    else if ((node=acceptDefspec3(s)) != NULL)
    {
        // REG node
        if (!m_identDB->addIdentifier(identifier, SymbolTable::info_t::T_REG))
        {
            error(s, "Identifier already exists!");
            return NULL;
        }
        return node;
    }
    return NULL;
}

AST::InputDeclaration* Parser::acceptDefspec1(state_t &s)
{
    // production: INPUT LPAREN INTEGER COMMA INTEGER RPAREN

    const uint32_t tokenList[] =
        {TOK_INPUT, TOK_LPAREN, TOK_INTEGER, TOK_COMMA, TOK_INTEGER, TOK_RPAREN, 0};

    state_t savestate = s;
    if (!matchList(s, tokenList))
    {
        s=savestate;
        return NULL;
    }

    AST::InputDeclaration* newNode = new AST::InputDeclaration();
    newNode->m_intBits  = atoi(getToken(s, -4).txt.c_str()); // first integer
    newNode->m_fracBits = atoi(getToken(s, -2).txt.c_str()); // second integer

    return newNode;
}


AST::CSDDeclaration *Parser::acceptDefspec2(state_t &s)
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

    AST::CSDDeclaration* newNode = new AST::CSDDeclaration();
    float    value = atof(getToken(s, -4).txt.c_str()); // first argument
    uint32_t bits  = atoi(getToken(s, -2).txt.c_str()); // second argument

    if (!convertToCSD(value, bits, newNode->m_csd))
    {
        error(s,"acceptDefspec2: cannot convert CSD");
    }

    //float msb = ceil(log10(fabs(static_cast<double>(csd.value)))/log10(2.0));
    //newNode->info.intBits = static_cast<int32_t>(msb+1); // add sign bit
    //newNode->info.fracBits = -csd.digits.back().power;
    //newNode->info.csd = csd;
    return newNode;
}

AST::RegDeclaration* Parser::acceptDefspec3(state_t &s)
{
    // production: REG LPAREN INTEGER COMMA INTEGER RPAREN

    const uint32_t tokenList[] =
        {TOK_REG, TOK_LPAREN, TOK_INTEGER, TOK_COMMA, TOK_INTEGER, TOK_RPAREN, 0};

    state_t savestate = s;
    if (!matchList(s, tokenList))
    {
        s=savestate;
        return NULL;
    }

    AST::RegDeclaration* newNode = new AST::RegDeclaration();
    newNode->m_intBits  = atoi(getToken(s, -4).txt.c_str()); // first integer
    newNode->m_fracBits = atoi(getToken(s, -2).txt.c_str()); // second integer
    return newNode;
}

AST::ASTNodeBase* Parser::acceptTruncate(state_t &s)
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

    AST::ASTNodeBase* exprNode = Parser::acceptExpr(s);
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

    AST::PrecisionModifier *newNode = new AST::PrecisionModifier(AST::PrecisionModifier::NodeTruncate);
    newNode->m_intBits  = intbits;
    newNode->m_fracBits = fracbits;
    newNode->m_argNode  = exprNode;

    return newNode;
}

AST::ASTNodeBase* Parser::acceptAssignment(state_t &s)
{
    // production: IDENT EQUAL expr SEMICOL
    state_t savestate = s;

    if (!match(s,TOK_IDENT))
    {
        s = savestate;
        return NULL;
    }

    if (!match(s,TOK_EQUAL))
    {
        error(s,"Expected '='");
        s = savestate;
        return NULL;
    }

    std::string identifier = getToken(s, -2).txt;

    AST::ASTNodeBase *exprNode = 0;
    if ((exprNode=acceptExpr(s)) == 0)
    {
        error(s,"Expression expected");
        s = savestate;
        return NULL;
    }

    if (!match(s, TOK_SEMICOL))
    {
        error(s,"Assignments must end with a semicolon.");
        s = savestate;
        delete exprNode;
        return NULL;
    }

    // if the identifier is unknown, it must be an
    // output variable as these are the only ones
    // not declared by the user.

    if (!m_identDB->hasIdentifier(identifier))
    {
        // no need to specify the precision as this will be
        // determined later on.
        m_identDB->addIdentifier(identifier, SymbolTable::info_t::T_OUTPUT);
    }

    // do type checking here.
    // we can only accept assignments to an output,
    // register or temporary variable.
    if (m_identDB->identIsType(identifier, SymbolTable::info_t::T_INPUT))
    {
        // cannot assign to type input
        error(s,"Cannot assign to input variables.");
        return NULL;
    }

    AST::Assignment *newNode = new AST::Assignment();
    newNode->m_identName = identifier;
    newNode->m_expr = exprNode;

    return newNode;
}

AST::ASTNodeBase* Parser::acceptExpr(state_t &s)
{
    // productions: term expr'
    //
    // the term is the first term
    // and must therefore be
    // added as the left leaf
    //

    state_t savestate = s;

    AST::ASTNodeBase *leftNode = 0;
    if ((leftNode=acceptTerm(s)) != NULL)
    {
        // the term is the left-hand size of the expr'
        // the right hand side and the operation node
        // itself still need to be found.
        //
        // note, exprAccentNode is never NULL
        // because of it's epsilon solution
        //
        AST::ASTNodeBase *exprAccentNode = acceptExprAccent(s, leftNode);
        return exprAccentNode;
    }
    s = savestate;
    return NULL;
}

AST::ASTNodeBase* Parser::acceptExprAccent(state_t &s, AST::ASTNodeBase *leftNode)
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

    AST::ASTNodeBase *topNode = 0;
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

AST::ASTNodeBase* Parser::acceptExprAccent1(state_t &s, AST::ASTNodeBase *leftNode)
{
    // production: - term expr'
    state_t savestate = s;

    if (!match(s, TOK_MINUS))
    {
        return NULL;
    }

    AST::ASTNodeBase *rightNode = 0;
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

    AST::Operation2 *operationNode = new AST::Operation2(AST::Operation2::NodeSub);
    operationNode->m_left = leftNode;
    operationNode->m_right = rightNode;

    // note: acceptExprAccent will never return NULL
    AST::ASTNodeBase *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}

AST::ASTNodeBase* Parser::acceptExprAccent2(state_t &s, AST::ASTNodeBase *leftNode)
{
    // production: + term expr'
    state_t savestate = s;

    if (!match(s, TOK_PLUS))
    {
        return NULL;
    }

    AST::ASTNodeBase *rightNode = 0;
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

    AST::Operation2 *operationNode = new AST::Operation2(AST::Operation2::NodeAdd);
    operationNode->m_left = leftNode;
    operationNode->m_right = rightNode;

    // note: acceptExprAccent will never return NULL
    AST::ASTNodeBase *headNode = acceptExprAccent(s, operationNode);
    return headNode;
}


AST::ASTNodeBase* Parser::acceptTerm(state_t &s)
{
    // production: factor term'
    state_t savestate = s;

    AST::ASTNodeBase *leftNode = 0;
    if ((leftNode=acceptFactor(s)) != NULL)
    {
        // the term is the left-hand size of the term'
        // the right hand side and the operation node
        // itself still need to be found.
        //
        // note, termAccentNode is never NULL
        // because of it's epsilon solution
        //
        AST::ASTNodeBase *termAccentNode = acceptTermAccent(s, leftNode);
        return termAccentNode;
    }

    s = savestate;
    return NULL;
}

AST::ASTNodeBase* Parser::acceptTermAccent(state_t &s, AST::ASTNodeBase *leftNode)
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

    AST::ASTNodeBase *topNode = 0;
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

AST::ASTNodeBase* Parser::acceptTermAccent1(state_t &s, AST::ASTNodeBase *leftNode)
{
    // production: * factor term'
    state_t savestate = s;

    if (!match(s, TOK_STAR))
    {
        return NULL;
    }

    AST::ASTNodeBase *rightNode = 0;
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

    AST::Operation2 *operationNode = new AST::Operation2(AST::Operation2::NodeMul);
    operationNode->m_left = leftNode;
    operationNode->m_right = rightNode;

    // note: acceptTermAccent will never return NULL
    AST::ASTNodeBase *headNode = acceptTermAccent(s, operationNode);
    return headNode;
}

AST::ASTNodeBase* Parser::acceptTermAccent2(state_t &s, AST::ASTNodeBase *leftNode)
{
    // production: / factor term'
    state_t savestate = s;

    if (!match(s, TOK_SLASH))
    {
        return NULL;
    }

    AST::ASTNodeBase *rightNode = 0;
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

    AST::Operation2 *operationNode = new AST::Operation2(AST::Operation2::NodeDiv);
    operationNode->m_left = leftNode;
    operationNode->m_right = rightNode;

    // note: acceptTermAccent will never return NULL
    AST::ASTNodeBase *headNode = acceptTermAccent(s, operationNode);
    return headNode;
}


AST::ASTNodeBase* Parser::acceptFactor(state_t &s)
{
    state_t savestate = s;

    // FUNCTION ( expr )
    AST::ASTNodeBase *factorNode = 0;

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
        AST::IntegerConstant* newNode = new AST::IntegerConstant();
        newNode->m_value = atoi(getToken(s, -1).txt.c_str());
        return newNode;
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
        AST::Identifier *newNode = new AST::Identifier();
        newNode->m_identName = getToken(s, -1).txt;
        return newNode; // IDENT
    }

    error(s, "Factor is not an integer, float, identifier or parenthesised expression.");
    return NULL;
}

AST::ASTNodeBase* Parser::acceptFactor1(state_t &s)
{    
    // production: FUNCTION ( expr )

    AST::ASTNodeBase *node = NULL;
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

AST::ASTNodeBase* Parser::acceptFactor2(state_t &s)
{
    // ( expr )

    state_t savestate = s;
    if (!match(s, TOK_LPAREN))
    {
        s = savestate;
        return NULL;
    }
    AST::ASTNodeBase *exprNode = 0;
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

AST::ASTNodeBase* Parser::acceptFactor3(state_t &s)
{
    // production: - factor
    state_t savestate = s;
    if (!match(s, TOK_MINUS))
    {
        s = savestate;
        return NULL;
    }

    AST::ASTNodeBase *factorNode = 0;
    if ((factorNode=acceptFactor(s)) == NULL)
    {
        s = savestate;
        return NULL;
    }

    // unary minus node
    AST::Operation1 *exprNode = new AST::Operation1(AST::Operation1::NodeUnaryMinus);
    exprNode->m_expr = factorNode;

    return exprNode;
}

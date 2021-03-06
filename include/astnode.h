/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Abstract syntax tree (AST) node definition

  Author: Niels A. Moseley

*/

#ifndef astnode_h
#define astnode_h

#include <string>
#include <list>
#include <stdint.h>
#include "csd.h"
#include "astvisitor.h"

/** variable related information */
struct varInfo
{
    varInfo() : fracBits(0),
        intBits(0),
        csdBits(0),
        csdFloat(0.0f),
        intVal(0)
    {
    }

    std::string     txt;        // identifier name, integer or float value.
    int32_t         fracBits;   // number of factional bits in INPUT definition
    int32_t         intBits;    // number of integer bits in INPUT defintion
    int32_t         csdBits;    // number of CSD factors/bits
    double          csdFloat;   // desired value of CSD coefficient
    int32_t         intVal;     // integer value
    csd_t           csd;        // CSD representation.
};

/** Abstract Syntax Tree Node with visitor pattern support */
class ASTNode
{
public:
    /** create an AST node */
    ASTNode() {}

    virtual ~ASTNode() {}

    /** accept an AST visitor for iteration */
    virtual void accept(AST::VisitorBase *visitor) = 0;
};


namespace AST
{

/** An AST node describing a collection of sequential statements */
class Statements : public ::ASTNode
{
public:
    Statements()
    {
    }

    virtual ~Statements() {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    std::list<ASTNode*> m_statements;   ///< collection of sequential statements
};

/** Declaration node base class holding information on an input var or constant name. */
class Declaration : public ::ASTNode
{
public:
    Declaration()
    {
    }

    std::string m_identName;    ///< name of the variable or constant
};

/** Identifier node */
class Identifier : public Declaration
{
public:
    Identifier()
    {
    }

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

};


/** Input declaration */
class InputDeclaration : public Declaration
{
public:
    InputDeclaration() :
        m_fracBits(0),
        m_intBits(0) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    int32_t m_fracBits;   // number of factional bits in INPUT definition
    int32_t m_intBits;    // number of integer bits in INPUT defintion
};


/** CSD declaration */
class CSDDeclaration : public Declaration
{
public:
    CSDDeclaration() {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    csd_t m_csd;
};


/** Precision modification node: RemoveLSB, ExtendLSB, RemoveMSB, ExtendMSB,
    Truncate, Saturate.
*/
class PrecisionModifier : public ::ASTNode
{
public:
    enum node_t
    {
        NodeUndefined = 0,
        NodeTruncate
    };

    explicit PrecisionModifier(node_t nodeType = NodeUndefined) :
        m_argNode(NULL),
        m_nodeType(nodeType),
        m_fracBits(0),
        m_intBits(0) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    ASTNode *m_argNode;         ///< pointer to argument AST.

    node_t  m_nodeType;         ///< type of precision modifier node
    int32_t m_fracBits;         ///< fractional bits parameter for truncate
    int32_t m_intBits;          ///< integer bits parameter for truncate
};


/** Assignment node */
class Assignment : public ::ASTNode
{
public:
    Assignment() : m_expr(NULL) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    ASTNode *m_expr;            ///< pointer to expression AST.
    std::string m_identName;    ///< name of output identifier
};


/** Integer constant */
class IntegerConstant : public ::ASTNode
{
public:
    IntegerConstant() : m_value(0) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    int32_t m_value;    ///< constant value
};

/** operation with two operands */
class Operation2 : public ::ASTNode
{
public:
    enum node_t
    {
        NodeUndefined = 0,
        NodeAdd,
        NodeSub,
        NodeMul,
        NodeDiv
    };

    explicit Operation2(node_t nodeType = NodeUndefined) :
        m_left(NULL),
        m_right(NULL),
        m_nodeType(nodeType) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    ASTNode *m_left;    ///< pointer to left expression AST.
    ASTNode *m_right;   ///< pointer to right expression AST.
    node_t m_nodeType;
};

/** unary functions */
class Operation1 : public ::ASTNode
{
public:
    enum node_t
    {
       NodeUndefined = 0,
       NodeUnaryMinus
    };

    explicit Operation1(node_t nodeType = NodeUndefined) :
        m_expr(NULL),
        m_nodeType(nodeType) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(AST::VisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    ASTNode *m_expr;    ///< pointer to expression AST.
    node_t m_nodeType;
};

} // namespace

#endif

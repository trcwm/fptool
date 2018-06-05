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

namespace AST
{

// pre-declarations
class Identifier;
class IntegerConstant;
class Statements;
class InputDeclaration;
class CSDDeclaration;
class RegDeclaration;
class PrecisionModifier;
class Assignment;
class Operation2;
class Operation1;


/** Visitor class for ASTNodes
    See: "Design Patterns: Elements of Reusable Object-Oriented Software"
*/
class ASTVisitorBase
{
public:
    virtual void visit(const Identifier *node) = 0;
    virtual void visit(const IntegerConstant *node) = 0;
    virtual void visit(const CSDDeclaration *node) = 0;
    virtual void visit(const RegDeclaration *node) = 0;
    virtual void visit(const Statements *node) = 0;
    virtual void visit(const InputDeclaration *node) = 0;
    virtual void visit(const PrecisionModifier *node) = 0;
    virtual void visit(const Assignment *node) = 0;
    virtual void visit(const Operation2 *node) = 0;
    virtual void visit(const Operation1 *node) = 0;
};

/** Abstract Syntax Tree Node with visitor pattern support */
class ASTNodeBase
{
public:
    /** create an AST node */
    ASTNodeBase() {}

    virtual ~ASTNodeBase() {}

    /** accept an AST visitor for iteration */
    virtual void accept(ASTVisitorBase *visitor) = 0;
};



/** An AST node describing a collection of sequential statements */
class Statements : public ASTNodeBase
{
public:
    Statements()
    {
    }

    virtual ~Statements() {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    std::list<ASTNodeBase*> m_statements;   ///< collection of sequential statements
};

/** Declaration node base class holding information on an input var or constant name. */
class Declaration : public ASTNodeBase
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
    virtual void accept(ASTVisitorBase *visitor) override
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
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    int32_t m_fracBits;   // number of factional bits in INPUT definition
    int32_t m_intBits;    // number of integer bits in INPUT defintion
};

/** Register declaration */
class RegDeclaration : public Declaration
{
public:
    RegDeclaration() :
        m_fracBits(0),
        m_intBits(0) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    int32_t m_fracBits;   // number of factional bits in REGISTER definition
    int32_t m_intBits;    // number of integer bits in REGISTER defintion
};

/** CSD declaration */
class CSDDeclaration : public Declaration
{
public:
    CSDDeclaration() {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    csd_t m_csd;
};



/** Precision modification node: RemoveLSB, ExtendLSB, RemoveMSB, ExtendMSB,
    Truncate, Saturate.
*/
class PrecisionModifier : public ASTNodeBase
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
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    ASTNodeBase *m_argNode;         ///< pointer to argument AST.

    node_t  m_nodeType;         ///< type of precision modifier node
    int32_t m_fracBits;         ///< fractional bits parameter for truncate
    int32_t m_intBits;          ///< integer bits parameter for truncate
};


/** Assignment node */
class Assignment : public ASTNodeBase
{
public:
    Assignment() : m_expr(NULL) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    ASTNodeBase *m_expr;        ///< pointer to expression AST.
    std::string m_identName;    ///< name of output identifier
};


/** Integer constant */
class IntegerConstant : public ASTNodeBase
{
public:
    IntegerConstant() : m_value(0) {}

    /** Accept a visitor by calling visitor->visit(this) */
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    int32_t m_value;    ///< constant value
};

/** operation with two operands */
class Operation2 : public ASTNodeBase
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
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    ASTNodeBase *m_left;    ///< pointer to left expression AST.
    ASTNodeBase *m_right;   ///< pointer to right expression AST.
    node_t m_nodeType;
};

/** unary functions */
class Operation1 : public ASTNodeBase
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
    virtual void accept(ASTVisitorBase *visitor) override
    {
        visitor->visit(this);
    }

    ASTNodeBase *m_expr;    ///< pointer to expression AST.
    node_t m_nodeType;
};



} // namespace

#endif

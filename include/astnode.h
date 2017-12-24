/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Abstract syntax tree (AST) node definition

  Author: Niels A. Moseley

*/

#ifndef astnode_h
#define astnode_h

#include <string>
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
  enum node_t {NodeUnknown, NodeHead,
               NodeStatement,
               NodeAssign,
               NodeInput, NodeCSD,
               NodeTemp,
               NodeAdd, NodeSub, NodeMul, NodeDiv,
               NodeUnaryMinus,
               NodeIdent,
               NodeInteger, NodeFloat,
               NodeTruncate
              };

    /** create an AST node */
    ASTNode(node_t nodeType = NodeUnknown)
    {
        left = 0;
        right = 0;
        type = nodeType;
    }

    /** accept an AST visitor for iteration */
    void accept(ASTVisitorBase *visitor) const
    {
        visitor->visit(this);
    }

    node_t  type;   // the type of the node
    varInfo info;   // variable related information

    ASTNode *left;
    ASTNode *right;
};

#endif

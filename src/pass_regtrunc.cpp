/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Clean the SSA list after the CSDMul pass

  1) remove re-interpret nodes.
  2) remove superfluous assignment nodes.

*/

#include "logging.h"
#include "pass_regtrunc.h"

using namespace SSA;

bool PassRegTrunc::execute(Program &ssa)
{
    doLog(LOG_INFO, "-------------------------\n");
    doLog(LOG_INFO, "  Running RegTrunc pass\n");
    doLog(LOG_INFO, "-------------------------\n");

    PassRegTrunc pass(ssa);

    // remove re-interpreted nodes
    for(auto statement : ssa.m_statements)
    {
        if (!statement->accept(&pass))
        {
            return false;
        }
    }

    ssa.applyPatches();
    //Note: we should not call updateOutputPrecision
    //      here as the removed reinterpret nodes
    //      will cause erronous results.
    //ssa.updateOutputPrecisions();
    return true;
}

bool PassRegTrunc::visit(const OpAssign *node)
{
    // if assignment is REG := expr,
    // check if the expr and REG have the same
    // precision. If not, insert a truncate node!

    RegOperand* lhs = dynamic_cast<RegOperand*>(node->m_lhs.get());
    IntermediateOperand* op  = dynamic_cast<IntermediateOperand*>(node->m_op.get());

    if ((lhs != NULL) && (op != NULL))
    {

        if ((node->m_lhs->m_intBits != node->m_op->m_intBits) ||
            (node->m_lhs->m_fracBits != node->m_op->m_fracBits))
        {
            doLog(LOG_WARN, "Inserting truncate node for REG assignment %s!\n",
                  node->m_lhs->m_identName.c_str());

            OpPatchBlock *patch = new OpPatchBlock(node);
            SharedOpPtr tmpOp = IntermediateOperand::createNewIntermediate();
            OpTruncate *truncNode = new OpTruncate(node->m_op, tmpOp,
                                                   lhs->m_intBits,
                                                   lhs->m_fracBits);


            OpAssign *newAssign = new OpAssign(tmpOp, node->m_lhs, true);
            patch->addStatement(truncNode);
            patch->addStatement(newAssign);
            m_ssa->m_operands.push_back(tmpOp);
            patchNode(node, patch); // replace the old assign node.
            return true;
        }
    }

    return true;
}

void PassRegTrunc::patchNode(const SSA::OperationBase *node, SSA::OpPatchBlock *patch)
{
    auto iter = std::find(m_ssa->m_statements.begin(), m_ssa->m_statements.end(), node);
    if (iter != m_ssa->m_statements.end())
    {
        *iter = patch;

        //FIXME: we should get rid of the node
        //       but we can't do that safely here as there
        //       could be iterators accessing it.
        //       perhaps we need a shared pointer?
        //       for now, we'll just let it leak.
        //delete node;
    }
}

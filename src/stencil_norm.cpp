#include "stencil_norm.h"

#include "ir.h"
#include "ir_rewriter.h"

using namespace std;

namespace simit {
namespace ir {

class NormalizeRowIndices : public IRRewriter {
  Var origin, linkSet;
  Expr pointSet;
  int dims;
  vector<Var> arguments;
  set<Var> outputTensors;
  vector<int> rowNormOff;

  vector<int> getLitInds(const SetRead *sr) {
    vector<int> litInds;
    for (auto e : sr->indices) {
      const Literal *lit = to<Literal>(e);
      iassert(isScalar(lit->type));
      iassert(isInt(lit->type));
      litInds.push_back(lit->getIntVal(0));
    }
    return litInds;
  }

  void visit(const SetRead *op) {
    iassert(rowNormOff.size() != 0)
        << "Set read visit not paired to output write";
    
    vector<int> litInds = getLitInds(op);
    vector<Expr> newInds;
    if (litInds.size() == rowNormOff.size()) { // point set
      for (unsigned i = 0; i < litInds.size(); ++i) {
        litInds[i] += rowNormOff[i];
        newInds.push_back(Literal::make(litInds[i]));
      }
    }
    else if (litInds.size() == rowNormOff.size()*2) { // link set
      for (unsigned i = 0; i < litInds.size(); ++i) {
        litInds[i] += rowNormOff[i%rowNormOff.size()];
        newInds.push_back(Literal::make(litInds[i]));
      }
    }
    else {
      ierror << "Offset applied to strange number of indices: "
             << rowNormOff.size() << " vs " << litInds.size();
    }
    expr = SetRead::make(op->set, newInds);
  }

  void visit(const TensorWrite *op) {
    // Must assume tensor write to output is always a var expr, otherwise we
    // require more complex code analysis
    if (!isa<VarExpr>(op->tensor)) {
      IRRewriter::visit(op);
      return;
    }
    Var out = to<VarExpr>(op->tensor)->var;
    if (!outputTensors.count(out)) {
      IRRewriter::visit(op);
      return;
    }

    if (op->indices.size() != 2) {
      IRRewriter::visit(op);
      return;
    }

    vector<Expr> indices;
    for (Expr ind : op->indices) {
      if (isa<VarExpr>(ind) &&
          to<VarExpr>(ind)->var == origin) {
        vector<Expr> zeros;
        for (int i = 0; i < dims; ++i) {
          zeros.push_back(Literal::make(0));
        }
        indices.push_back(SetRead::make(pointSet, zeros));
      }
      else {
        iassert(isa<SetRead>(ind));
        indices.push_back(ind);
      }
    }

    iassert(isa<SetRead>(indices[0]));
    iassert(isa<SetRead>(indices[1]));
    const SetRead *rowRead= to<SetRead>(indices[0]);
    const SetRead *colRead= to<SetRead>(indices[1]);
    vector<int> rowIndices = getLitInds(rowRead);
    vector<int> colIndices = getLitInds(colRead);

    iassert(rowNormOff.size() == 0);
    // Negate row indices for offset in this statement
    for (int rind : rowIndices) {
      rowNormOff.push_back(-rind);
    }

    // Replace our set reads, and rewrite the whole statement
    vector<Expr> newIndices = {rewrite(rowRead), rewrite(colRead)};
    stmt = TensorWrite::make(op->tensor, newIndices, rewrite(op->value), op->cop);
    rowNormOff.clear();
  }

  void visit(const Func *op) {
    if (op->getKind() != Func::Internal) {
      IRRewriter::visit(op);
      return;
    }

    // TODO: Bit of a sketchy pattern-match. Need map context to do this properly.
    arguments = op->getArguments();
    // Stencil kernels have at least 2 arguments: a node and a link set
    if (arguments.size() < 2) {
      IRRewriter::visit(op);
      return;
    }

    if (!arguments[0].getType().isElement() ||
        !arguments[arguments.size()-1].getType().isSet()) {
      IRRewriter::visit(op);
      return;
    }
    origin = arguments[0];
    linkSet = arguments[arguments.size()-1];
    if (linkSet.getType().toSet()->kind != SetType::LatticeLink) {
      IRRewriter::visit(op);
      return;
    }
    pointSet = linkSet.getType().toSet()->latticePointSet.getSet();
    dims = linkSet.getType().toSet()->dimensions;
    
    for (auto &res : op->getResults()) {
      outputTensors.insert(res);
    }

    Stmt body = rewrite(op->getBody());
    func = Func(*op, body);
    func.setStorage(op->getStorage());
  }
};

Func normalizeRowIndices(Func func) {
  func = NormalizeRowIndices().rewrite(func);
  return func;
}

}} // namespace simit::ir

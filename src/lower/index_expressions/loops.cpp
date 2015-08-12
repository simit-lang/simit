#include "loops.h"

#include <stack>
#include "ir_visitor.h"

using namespace std;

namespace simit {
namespace ir {

// class IndexVariableLoop
struct IndexVariableLoop::Content {
  IndexVar indexVar;
  Var inductionVar;
  IndexVariableLoop linkedLoop;
};

IndexVariableLoop::IndexVariableLoop() {
}

IndexVariableLoop::IndexVariableLoop(const IndexVar &indexVar)
    : IndexVariableLoop(indexVar, IndexVariableLoop()) {
}

IndexVariableLoop::IndexVariableLoop(const IndexVar &indexVar,
                                     IndexVariableLoop linkedLoop)
    : content(new Content) {
  content->indexVar = indexVar;
  content->inductionVar = Var(indexVar.getName(), Int);
  content->linkedLoop = linkedLoop;
}

const IndexVar &IndexVariableLoop::getIndexVar() const {
  return content->indexVar;
}

const Var &IndexVariableLoop::getInductionVar() const {
  return content->inductionVar;
}

bool IndexVariableLoop::isLinked() const {
  return content->linkedLoop.defined();
}

IndexVariableLoop IndexVariableLoop::getLinkedLoop() const {
  return content->linkedLoop;
}


// class TensorIndexVar
TensorIndexVar::TensorIndexVar(Var inductionVar, Var sourceVar, Var tensor,
                 unsigned sourceDim, unsigned sinkDim) {
  this->sinkVar = Var(inductionVar.getName() + tensor.getName(), Int);
  this->sourceVar = sourceVar;
  this->coordinateVar =
      Var(sourceVar.getName()+inductionVar.getName()+tensor.getName(), Int);
  this->tensorIndex = TensorIndex(tensor, sourceDim);
}

Expr TensorIndexVar::loadCoordinate(int offset) const {
  Expr sourceExpr = (offset == 0) ? getSourceVar() : getSourceVar() + offset;
  return TensorIndexRead::make(getTensorIndex(), sourceExpr,
                               TensorIndexRead::Sources);
}

Expr TensorIndexVar::loadSink() const {
  return TensorIndexRead::make(getTensorIndex(), getCoordinateVar(),
                               TensorIndexRead::Sinks);
}

Stmt TensorIndexVar::initCoordinateVar() const {
  return AssignStmt::make(getCoordinateVar(), loadCoordinate());;
}

Stmt TensorIndexVar::initSinkVar() const {
  return AssignStmt::make(getSinkVar(), loadSink());
}


ostream &operator<<(ostream &os, const TensorIndexVar &tiv) {
  os << tiv.sinkVar
     << " in "      << tiv.tensorIndex
     << ".sinks["   << tiv.coordinateVar
     << " in "      << tiv.tensorIndex
     << ".sources[" << tiv.sourceVar << "]]";
  return os;
}


// class SubsetLoop
std::ostream &operator<<(std::ostream &os, const SubsetLoop &ssl) {
  os << "foreach zip(" << util::join(ssl.getTensorIndexVars()) << ")";
  return os;
}


// Free functions
vector<SubsetLoop> createSubsetLoops(Expr target,
                                     const IndexExpr *indexExpression,
                                     IndexVariableLoop loop) {
  class CreateSubsetLoopVisitor : public IRVisitor {
  public:
    CreateSubsetLoopVisitor(Expr target, IndexVariableLoop loop) {
      iassert(loop.isLinked());
      this->target = target;
      this->indexVar = loop.getIndexVar();
      this->inductionVar = loop.getInductionVar();
      this->linkedIndexVar = loop.getLinkedLoop().getIndexVar();
      this->linkedInductionVar = loop.getLinkedLoop().getInductionVar();
    }

    vector<SubsetLoop> createSubsetLoops(const IndexExpr *indexExpression) {
      return createSubsetLoops(Expr(indexExpression));
    }

  private:
    Expr target;

    IndexVar indexVar;
    Var inductionVar;
    IndexVar linkedIndexVar;
    Var linkedInductionVar;

    vector<SubsetLoop> subsetLoops;

    vector<SubsetLoop> createSubsetLoops(Expr expr) {
      expr.accept(this);
      vector<SubsetLoop> subsetLoops = this->subsetLoops;
      this->subsetLoops.clear();
      return subsetLoops;
    }

    /// Concatenates the subset loops in a an b
    inline vector<SubsetLoop> unionMerge(const vector<SubsetLoop> &a,
                                         const vector<SubsetLoop> &b) {
      vector<SubsetLoop> c;
      c.reserve(a.size() + b.size());

      for (auto &as : a) {
        c.push_back(as);
      }
      for (auto &bs : b) {
        c.push_back(bs);
      }

      return c;
    }

    /// Takes the set product of the subset loops in a and b
    inline vector<SubsetLoop> intersectionMerge(const vector<SubsetLoop> &a,
                                                const vector<SubsetLoop> &b) {
      vector<SubsetLoop> c;
      c.reserve(a.size() * b.size());

      for (auto &as : a) {
        vector<TensorIndexVar> ativars = as.getTensorIndexVars();
        for (auto &bs : b) {
          vector<TensorIndexVar> btivars = bs.getTensorIndexVars();

          vector<TensorIndexVar> ctivars;
          ctivars.reserve(ativars.size() + btivars.size());

          for (auto &ativar : ativars) {
            ctivars.push_back(ativar);
          }
          for (auto &btivar : btivars) {
            ctivars.push_back(btivar);
          }

          c.push_back(SubsetLoop(ctivars));
        }
      }

      return c;
    }

    void visit(const Add *op) {
      this->subsetLoops = unionMerge(createSubsetLoops(op->a),
                                     createSubsetLoops(op->b));
    }

    void visit(const Sub *op) {
      this->subsetLoops = unionMerge(createSubsetLoops(op->a),
                                     createSubsetLoops(op->b));
    }

    void visit(const Mul *op) {
      this->subsetLoops = intersectionMerge(createSubsetLoops(op->a),
                                            createSubsetLoops(op->b));
    }

    void visit(const Div *op) {
      this->subsetLoops = intersectionMerge(createSubsetLoops(op->a),
                                            createSubsetLoops(op->b));
    }

    void visit(const IndexedTensor *indexedTensor) {
      const vector<IndexVar> &indexVars = indexedTensor->indexVars;
      iassert(isa<VarExpr>(indexedTensor->tensor))
          << "at this point the index expressions should have been flattened";
      Var tensor = to<VarExpr>(indexedTensor->tensor)->var;

      TensorIndexVar tensorIndexVar(inductionVar, linkedInductionVar, tensor,
                                    util::locate(indexVars,linkedIndexVar),
                                    util::locate(indexVars,indexVar));
      subsetLoops = {SubsetLoop({tensorIndexVar})};
    }
  };
  CreateSubsetLoopVisitor visitor(target, loop);
  return visitor.createSubsetLoops(indexExpression);
}

}}

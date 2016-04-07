#include "loops.h"

#include <stack>
#include "ir_visitor.h"
#include "util/collections.h"

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

IndexVariableLoop::IndexVariableLoop(const IndexVar& indexVar)
    : IndexVariableLoop(indexVar, IndexVariableLoop()) {
}

IndexVariableLoop::IndexVariableLoop(const IndexVar& indexVar,
                                     IndexVariableLoop linkedLoop)
    : content(new Content) {
  content->indexVar = indexVar;
  content->inductionVar = Var(indexVar.getName(), Int);
  content->linkedLoop = linkedLoop;
}

const IndexVar& IndexVariableLoop::getIndexVar() const {
  return content->indexVar;
}

const Var& IndexVariableLoop::getInductionVar() const {
  return content->inductionVar;
}

bool IndexVariableLoop::isLinked() const {
  return content->linkedLoop.defined();
}

const IndexVariableLoop& IndexVariableLoop::getLinkedLoop() const {
  return content->linkedLoop;
}

std::ostream& operator<<(std::ostream& os, const IndexVariableLoop& ivl) {
  if (ivl.getLinkedLoop().defined()) {
    os << ivl.getLinkedLoop().getIndexVar() << "->";
  }
  os << ivl.getIndexVar();
  return os;
}

// class TensorIndexVar
TensorIndexVar::TensorIndexVar(string inductionVarName, string tensorName,
                               Var sourceVar, TensorIndex tensorIndex) {
  string sinkName = inductionVarName + tensorName;
  this->sourceVar = sourceVar;
  this->coordinateVar = Var(sourceVar.getName() + sinkName, Int);
  this->sinkVar = Var(sinkName, Int);
  this->tensorIndex = tensorIndex;
}

Expr TensorIndexVar::loadCoord(int offset) const {
  Expr sourceExpr = (offset == 0) ? getSourceVar() : getSourceVar() + offset;
  return Load::make(getTensorIndex().getCoordArray(), sourceExpr);
}

Expr TensorIndexVar::loadSink() const {
  return Load::make(getTensorIndex().getSinkArray(), getCoordVar());
}

Stmt TensorIndexVar::initCoordVar() const {
  return AssignStmt::make(getCoordVar(), loadCoord());;
}

Stmt TensorIndexVar::initSinkVar() const {
  return initSinkVar(getSinkVar());
}

Stmt TensorIndexVar::initSinkVar(const Var& sinkVar) const {
  return AssignStmt::make(sinkVar, loadSink());
}


ostream& operator<<(ostream& os, const TensorIndexVar& tiv) {
  os << tiv.sinkVar
     << " in "      << tiv.tensorIndex
     << ".sinks["   << tiv.coordinateVar
     << " in "      << tiv.tensorIndex
     << ".coordinates[" << tiv.sourceVar << "]]";
  return os;
}


// class SubsetLoop
std::ostream& operator<<(std::ostream& os, const SubsetLoop& ssl) {
  os << "foreach zip(";
  if (ssl.getTensorIndexVars().size() > 0) {
    os << ssl.getTensorIndexVars()[0].getCoordVar();
  }
  for (auto& tiVar : util::excludeFirst(ssl.getTensorIndexVars())) {
    os << ", " << tiVar.getCoordVar();
  }
  os << ")" << endl;
  os << " " << ssl.getCompoundOperator() << "= " << ssl.getComputeExpression();
  return os;
}


// Free functions
class CreateSubsetLoopVisitor : public IRVisitor {
public:
  CreateSubsetLoopVisitor(IndexVariableLoop loop, Environment* environment,
                          Storage* storage) {
    iassert(loop.isLinked());
    this->indexVar = loop.getIndexVar();
    this->inductionVar = loop.getInductionVar();
    this->linkedIndexVar = loop.getLinkedLoop().getIndexVar();
    this->linkedInductionVar = loop.getLinkedLoop().getInductionVar();
    this->environment = environment;
    this->storage = storage;
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

  Environment* environment;
  Storage* storage;

  vector<SubsetLoop> subsetLoops;

  vector<SubsetLoop> createSubsetLoops(Expr expr) {
    expr.accept(this);
    vector<SubsetLoop> subsetLoops = this->subsetLoops;
    this->subsetLoops.clear();
    return subsetLoops;
  }

  void visit(const Neg* op) {
    vector<SubsetLoop> a = createSubsetLoops(op->a);
    vector<SubsetLoop> c;
    c.reserve(a.size());
    for (auto& as : a) {
      Expr aExpr = as.getComputeExpression();
      c.push_back(SubsetLoop(as.getTensorIndexVars(), Neg::make(aExpr), op));
    }
    this->subsetLoops = c;
  }

  /// Concatenates the subset loops in a an b
  inline vector<SubsetLoop> unionMerge(const vector<SubsetLoop>& a,
                                       const vector<SubsetLoop>& b,
                                       CompoundOperator cop) {
    vector<SubsetLoop> c;
    c.reserve(a.size() + b.size());

    c.insert(c.end(), a.begin(), a.end());
    c.insert(c.end(), b.begin(), b.end());

    c[0].setCompoundOperator(CompoundOperator::None);
    for (size_t i=1; i < c.size(); ++i) {
      c[i].setCompoundOperator(cop);
    }

    return c;
  }

  /// Takes the set product of the subset loops in a and b
  vector<SubsetLoop> intersectionMerge(const vector<SubsetLoop>& a,
                                       const vector<SubsetLoop>& b,
                                       function<Expr(Expr,Expr)> op) {
    vector<SubsetLoop> c;
    c.reserve(a.size() * b.size());

    for (const SubsetLoop& as : a) {
      vector<TensorIndexVar> ativars = as.getTensorIndexVars();
      Expr aComputeExpr = as.getComputeExpression();

      for (const SubsetLoop& bs : b) {
        vector<TensorIndexVar> btivars = bs.getTensorIndexVars();
        Expr bComputeExpr = bs.getComputeExpression();

        vector<TensorIndexVar> ctivars;
        ctivars.reserve(ativars.size() + btivars.size());

        ctivars.insert(ctivars.end(), ativars.begin(), ativars.end());
        ctivars.insert(ctivars.end(), btivars.begin(), btivars.end());

        Expr iexpr = op(as.getIndexExpression(), bs.getIndexExpression());
        SubsetLoop cs = SubsetLoop(ctivars,op(aComputeExpr,bComputeExpr),iexpr);

        CompoundOperator ac = as.getCompoundOperator();
        CompoundOperator bc = bs.getCompoundOperator();
        if (ac != CompoundOperator::None || bc != CompoundOperator::None) {
          // TODO: Handle (B+C)(D-E) = BD-BE+CD-CE
          //       In this case CE will have two compound operators but +- = -
          tassert(ac == CompoundOperator::None || bc== CompoundOperator::None ||
                  ac==bc);
          cs.setCompoundOperator(ac);
        }

        c.push_back(cs);
      }
    }

    return c;
  }

  template <class T>
  inline void visitBinaryUnionOperator(const T* op, CompoundOperator cop) {
    this->subsetLoops = unionMerge(createSubsetLoops(op->a),
                                   createSubsetLoops(op->b),
                                   cop);
  }

  template <class T>
  inline void visitBinaryIntersectionOperator(const T* op) {
    this->subsetLoops = intersectionMerge(createSubsetLoops(op->a),
                                          createSubsetLoops(op->b),
                                          T::make);
  }

  void visit(const Add* op) {
    visitBinaryUnionOperator(op, CompoundOperator::Add);
  }

  void visit(const Sub* op) {
    not_supported_yet;
    /// TODO: Add support for CompoundOperator::Sub
    //      this->subsetLoops = unionMerge(createSubsetLoops(op->a),
    //                                     createSubsetLoops(op->b),
    //                                     CompoundOperator::Sub);
  }

  void visit(const Mul *op) {
    visitBinaryIntersectionOperator(op);
  }

  void visit(const Div *op) {
    visitBinaryIntersectionOperator(op);
  }

  void visit(const IndexedTensor *indexedTensor) {
    const vector<IndexVar>& indexVars = indexedTensor->indexVars;
    iassert(isa<VarExpr>(indexedTensor->tensor))
        << "at this point the index expressions should have been flattened";
    Var tensor = to<VarExpr>(indexedTensor->tensor)->var;

    TensorStorage& ts = storage->getStorage(tensor);
    unsigned sourceDim = util::locate(indexVars,linkedIndexVar);
    unsigned sinkDim   = util::locate(indexVars,indexVar);
    TensorIndex ti;
    if (!ts.hasTensorIndex(sourceDim, sinkDim)) {
      if (environment->hasExtern(tensor.getName())) {
        ts.addTensorIndex(tensor, sourceDim, sinkDim);
        ti = ts.getTensorIndex(sourceDim, sinkDim);
        environment->addExternMapping(tensor, ti.getCoordArray());
        environment->addExternMapping(tensor, ti.getSinkArray());
      }
      else if (ts.hasPathExpression()) {
        const pe::PathExpression pexpr = ts.getPathExpression();
        if (!environment->hasTensorIndex(pexpr)) {
          environment->addTensorIndex(pexpr, tensor);
        }
        ti = environment->getTensorIndex(pexpr);
      }
      else {
        // No subset loops to create?
        not_supported_yet;
      }
    }
    else {
      ti = ts.getTensorIndex(sourceDim, sinkDim);
    }

    TensorIndexVar tensorIndexVar(inductionVar.getName(), tensor.getName(),
                                  linkedInductionVar, ti);

    Expr tensorLoad = Load::make(indexedTensor->tensor,
                                 tensorIndexVar.getCoordVar());
    subsetLoops = {SubsetLoop({tensorIndexVar}, tensorLoad, indexedTensor)};
  }
};

vector<SubsetLoop> createSubsetLoops(const IndexExpr* indexExpr,
                                     IndexVariableLoop loop,
                                     Environment* environment,
                                     Storage* storage) {
  CreateSubsetLoopVisitor visitor(loop, environment, storage);
  return visitor.createSubsetLoops(indexExpr);
}

}}

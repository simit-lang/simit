#include "storage.h"

#include <memory>

#include "ir.h"
#include "ir_visitor.h"
#include "path_expressions.h"
#include "tensor_index.h"
#include "path_expression_analysis.h"
#include "util/collections.h"

using namespace std;

namespace simit {
namespace ir {

// class TensorStorage
struct TensorStorage::Content {
  Kind kind;

  /// The target set that was used to assemble the system if the tensor is
  /// stored on a system, undefined otherwise.
  Expr systemTargetSet;
  Expr systemStorageSet;

  /// The stencil assembly information for stencil-type storage
  // string-based ref to func, to avoid dangling references to a rewritten Func
  string assemblyFunc;
  // string-based ref to output var, again avoiding dangling references
  string targetVar;
  // fully resolve stencil structure: map from list of offsets to
  // ``stencil index'', i.e. a labelling of the stencil offsets from 0 to
  // the total size, defining an ordering of matrix elements in memory
  bool stencilDefined;
  Stencil stencil;

  pe::PathExpression pathExpression;
  map<pair<unsigned,unsigned>, TensorIndex> tensorIndices;
};

TensorStorage::TensorStorage() : TensorStorage(Kind::Undefined) {
}

TensorStorage::TensorStorage(Kind kind) : content(new Content) {
  content->kind = kind;
}

TensorStorage::TensorStorage(const Expr &targetSet)
    : TensorStorage(Kind::Diagonal) {
  content->systemTargetSet = targetSet;
}

TensorStorage::TensorStorage(const Expr &targetSet, const Expr &storageSet)
    : TensorStorage(Kind::Indexed) {
  content->systemTargetSet = targetSet;
  content->systemStorageSet = storageSet;
}

TensorStorage::TensorStorage(string assemblyFunc, string targetVar,
                             const Expr &targetSet,
                             const Expr &throughSet)
    : TensorStorage(Kind::Stencil) {
  content->assemblyFunc = assemblyFunc;
  content->targetVar = targetVar;
  content->systemTargetSet = targetSet;
  content->systemStorageSet = throughSet;
}


TensorStorage::Kind TensorStorage::getKind() const {
  return content->kind;
}

bool TensorStorage::isDense() const {
  return getKind() == Kind::Dense;
}

bool TensorStorage::isSystem() const {
  switch (getKind()) {
    case Kind::Dense:
      return false;
    case Kind::Indexed:
    case Kind::Diagonal:
    case Kind::MatrixFree:
    case Kind::Stencil:
      return true;
    case Kind::Undefined:
      ierror;
  }
  return false;
}

bool TensorStorage::hasPathExpression() const {
  return content->pathExpression.defined();
}

const pe::PathExpression& TensorStorage::getPathExpression() const {
  return content->pathExpression;
}

void TensorStorage::setPathExpression(const pe::PathExpression& pathExpression){
  content->pathExpression = pathExpression;
}

std::string TensorStorage::getStencilFunc() const {
  iassert(content->kind == Kind::Stencil);
  return content->assemblyFunc;
}

std::string TensorStorage::getStencilVar() const {
  iassert(content->kind == Kind::Stencil);
  return content->targetVar;
}

bool TensorStorage::hasStencil() const {
  return content->stencilDefined;
}

const Stencil& TensorStorage::getStencil() const {
  return content->stencil;
}

void TensorStorage::setStencil(const Stencil& stencil) {
  content->stencil = stencil;
  content->stencilDefined = true;
}

bool TensorStorage::hasTensorIndex(unsigned sourceDim, unsigned sinkDim) const {
  return util::contains(content->tensorIndices, {sourceDim, sinkDim});
}

const TensorIndex& TensorStorage::getTensorIndex(unsigned sourceDim,
                                                 unsigned sinkDim) const {
  iassert(hasTensorIndex(sourceDim,sinkDim));
  tassert(sourceDim == 0 && sinkDim == 1)
      << "Only currently support row->col indices";
  return content->tensorIndices.at({sourceDim,sinkDim});
}

void TensorStorage::addTensorIndex(Var tensor, unsigned srcDim,
                                   unsigned sinkDim) {
  iassert(!hasTensorIndex(srcDim, sinkDim));
  content->tensorIndices.insert({{srcDim,sinkDim},
      TensorIndex(tensor.getName()+"_index", pe::PathExpression())});

}

const Expr &TensorStorage::getSystemTargetSet() const {
  iassert(isSystem()) << "System storages require the target set be provided";
  return content->systemTargetSet;
}

const Expr &TensorStorage::getSystemStorageSet() const {
  iassert(isSystem()) << "System storages require the storage set be provided";
  return content->systemStorageSet;
}

std::ostream &operator<<(std::ostream &os, const TensorStorage &ts) {
  switch (ts.getKind()) {
    case TensorStorage::Kind::Undefined:
      os << "Undefined";
      break;
    case TensorStorage::Kind::Dense:
      os << "Dense";
      break;
    case TensorStorage::Kind::Indexed:
      os << "Indexed";
      break;
    case TensorStorage::Kind::Stencil:
      os << "Stencil";
      break;
    case TensorStorage::Kind::Diagonal:
      os << "Diagonal";
      break;
    case TensorStorage::Kind::MatrixFree:
      os << "Matrix-Free";
      break;
    default:
      unreachable;
  }
  return os;
}

// class Storage
struct Storage::Content {
  std::map<Var,TensorStorage> storage;
};

Storage::Storage() : content(new Content) {
}

void Storage::add(const Var &tensor, TensorStorage tensorStorage) {
  content->storage[tensor] = tensorStorage;
}

void Storage::add(const Storage &other) {
  for (auto &var : other) {
    // iassert(!hasStorage(var)) << "Variable " << var << " already has storage";
    // TEMP: Hack to avoid issues with multiple copies of const global tensor
    if (!hasStorage(var)) {
      add(var, other.getStorage(var));
    }
  }
}

bool Storage::hasStorage(const Var &tensor) const {
  return content->storage.find(tensor) != content->storage.end();
}

TensorStorage &Storage::getStorage(const Var &tensor) {
  iassert(hasStorage(tensor))
      << " no tensor storage specified for " << util::quote(tensor);
  return content->storage.at(tensor);
}

const TensorStorage &Storage::getStorage(const Var &tensor) const {
  return const_cast<Storage*>(this)->getStorage(tensor);
}

struct Storage::Iterator::Content {
  std::map<Var,TensorStorage>::iterator it;
};

Storage::Iterator::Iterator(Storage::Iterator::Content *content)
    : content(content) {
}

Storage::Iterator::~Iterator() {
  delete content;
}

const Var &Storage::Iterator::operator*() {
  return content->it->first;
}

const Var *Storage::Iterator::operator->() {
  return &content->it->first;
}

Storage::Iterator& Storage::Iterator::operator++() {
  content->it++;
  return *this;
}

bool operator!=(const Storage::Iterator &l, const Storage::Iterator &r) {
  return l.content->it != r.content->it;
}

std::ostream &operator<<(std::ostream &os, const Storage &storage) {
  Storage::Iterator it = storage.begin();
  Storage::Iterator end = storage.end();
  if (it != end) {
    os << *it << " : " << storage.getStorage(*it);
    ++it;
  }
  for (; it != end; ++it) {
    os << std::endl << *it << " : " << storage.getStorage(*it);
  }
  return os;
}

Storage::Iterator Storage::begin() const {
  auto content = new Storage::Iterator::Content;
  content->it = this->content->storage.begin();
  return Storage::Iterator(content);
}

Storage::Iterator Storage::end() const {
  auto content = new Storage::Iterator::Content;
  content->it = this->content->storage.end();
  return Storage::Iterator(content);
}

// Free functions
class GetStorageVisitor : public IRVisitor {
public:
  GetStorageVisitor(Storage *storage) : storage(storage) {}

  void get(Func func) {
    for (auto &global : func.getEnvironment().getConstants()) {
      if (global.first.getType().isTensor()) {
        determineStorage(global.first);
      }
    }

    for (auto &arg : func.getArguments()) {
      if (arg.getType().isTensor()) {
        determineStorage(arg);
      }
    }

    for (auto &res : func.getResults()) {
      if (res.getType().isTensor()) {
        determineStorage(res);
      }
    }

    func.accept(this);
  }

  void get(Stmt stmt) {
    stmt.accept(this);
  }

private:
  Storage *storage;
  PathExpressionBuilder peBuilder;

  using IRVisitor::visit;

  void visit(const VarDecl *op) {
    Var var = op->var;
    Type type = var.getType();
    //iassert(!storage->hasStorage(var)) << "Redeclaration of variable " << var;
    if (type.isTensor() && !isScalar(type)) {
      determineStorage(var);
    }
  }

  void visit(const Map *op) {
    // If the map target set is not an edge set, then matrices are diagonal.
    // Otherwise, the matrices are indexed with a path expression
    Type targetType = op->target.type();
    iassert(targetType.isSet());
    if (targetType.toSet()->getCardinality() == 0) {
      for (const Var& var : op->vars) {
        iassert(var.getType().isTensor());
        const TensorType* type = var.getType().toTensor();

        if (type->order() < 2) {
          // Dense
          storage->add(var, TensorStorage(TensorStorage::Kind::Dense));
        }
        else if (op->through.defined()) {
          // Stencil
          storage->add(var, TensorStorage(
              op->function.getName(), var.getName(), op->target, op->through));
        }
        else {
          // Indexed
          storage->add(var, TensorStorage(op->target));
        }
      }
    }
    else {
      peBuilder.computePathExpression(op);

      for (const Var& var : op->vars) {
        Type type = var.getType();
        if (type.isTensor() && !isScalar(type)) {
          // For now we'll store all assembled vectors as dense and other tensors
          // as system reduced
          TensorStorage tensorStorage;
          const TensorType* tensorType = type.toTensor();
          if (tensorType->order() == 1) {
            tensorStorage = TensorStorage(TensorStorage::Kind::Dense);
          }
          else {
            if (op->neighbors.defined()) {
              tensorStorage = TensorStorage(op->target, op->neighbors);

              // Add path expression
              tassert(tensorType->order() == 2)
              << "tensor has order " << tensorType->order()
              << ", while we only currently supports sparse matrices";
              tensorStorage.setPathExpression(peBuilder.getPathExpression(var));
            }
            else {
              tensorStorage = TensorStorage(op->target);
            }
          }
          iassert(tensorStorage.getKind() != TensorStorage::Kind::Undefined);
          storage->add(var, tensorStorage);
        }
      }
    }
  }

  void visit(const AssignStmt *op) {
    Var var = op->var;
    Type type = var.getType();
    Type rhsType = op->value.type();

    if (!isScalar(type)) {
      iassert(type.isTensor());
      const TensorType *ttype = type.toTensor();
      
      // Element tensor and system vectors are dense.
      if (isElementTensorType(ttype) || ttype->order() <= 1) {
        if (!storage->hasStorage(var)) {
          determineStorage(var);
        }
      }
      // System matrices
      else {
        // assume system reduced storage
        determineStorage(var, op->value);

        if (isa<IndexExpr>(op->value)) {
          peBuilder.computePathExpression(var, to<IndexExpr>(op->value));
          pe::PathExpression pexpr = peBuilder.getPathExpression(var);
          storage->getStorage(var).setPathExpression(pexpr);
        } else if (isa<VarExpr>(op->value) && rhsType.isTensor()) {
          pe::PathExpression pexpr = storage->getStorage(to<VarExpr>(op->value)->var).getPathExpression();
          storage->getStorage(var).setPathExpression(pexpr);
        }
      }
    }
  }

  void visit(const TensorWrite *op) {
    if (isa<VarExpr>(op->tensor)) {
      const Var &var = to<VarExpr>(op->tensor)->var;
      Type type = var.getType();
      if (type.isTensor() && !isScalar(type) && !storage->hasStorage(var)) {
        determineStorage(var);
      }
    }
  }

  void determineStorage(Var var, Expr rhs=Expr()) {
    // Scalars don't need storage
    if (isScalar(var.getType())) return;

    // If all dimensions are ranges then we choose dense row major. Otherwise,
    // we choose system reduced storage order (for now).
    Type type = var.getType();
    iassert(type.isTensor());
    const TensorType *ttype = type.toTensor();

    TensorStorage tensorStorage;

    // Element tensor and system vectors are dense.
    if (isElementTensorType(ttype) || ttype->order() == 1 || !rhs.defined()) {
      tensorStorage = TensorStorage(TensorStorage::Kind::Dense);
    }
    // System matrices
    else {
      // find the leaf Vars in the RHS expression
      class LeafVarsVisitor : public IRVisitor {
      public:
        std::set<Var> vars;
        using IRVisitor::visit;
        void visit(const VarExpr *op) {
          vars.insert(op->var);
        }
      };
      LeafVarsVisitor leafVars;
      rhs.accept(&leafVars);

      // When creating a matrix by combining two matrices, the new matrix gets
      // the storage with higher priority from the inputs.
      // E.g. if one of the input variables to the RHS expression is dense then
      // the output becomes dense.
      static map<TensorStorage::Kind, unsigned> priorities = {
        {TensorStorage::Kind::Dense,      4},
        {TensorStorage::Kind::Indexed,    3},
        {TensorStorage::Kind::Diagonal,   2},
        {TensorStorage::Kind::MatrixFree, 1},
        {TensorStorage::Kind::Undefined,  0}
      };

      for (Var operand : leafVars.vars) {
        if (isScalar(operand.getType())) {
          continue;
        }

        iassert(storage->hasStorage(operand))
            << operand << "does not have a storage descriptor";

        const TensorStorage operandStorage  = storage->getStorage(operand);
        auto operandStorageKind = operandStorage.getKind();
        auto tensorStorageKind = tensorStorage.getKind();
        if (priorities[operandStorageKind] > priorities[tensorStorageKind]) {
          switch (operandStorage.getKind()) {
            case TensorStorage::Kind::Dense:
            case TensorStorage::Kind::MatrixFree:
              tensorStorage = operandStorage.getKind();
              break;
            case TensorStorage::Kind::Indexed:
              tensorStorage =
                  TensorStorage(operandStorage.getSystemTargetSet(),
                                operandStorage.getSystemStorageSet());
              break;
            case TensorStorage::Kind::Stencil:
              tensorStorage =
                  TensorStorage(operandStorage.getStencilFunc(),
                                operandStorage.getStencilVar(),
                                operandStorage.getSystemTargetSet(),
                                operandStorage.getSystemStorageSet());
              if (operandStorage.hasStencil()) {
                tensorStorage.setStencil(operandStorage.getStencil());
              }
              break;
            case TensorStorage::Kind::Diagonal:
              tensorStorage =
                  TensorStorage(operandStorage.getSystemTargetSet());
              break;
            case TensorStorage::Kind::Undefined:
              unreachable;
              break;
            default:
              unreachable;
          }
        }
      }
    }

    if (tensorStorage.getKind() != TensorStorage::Kind::Undefined) {
      storage->add(var, tensorStorage);
    }
  }
};

Storage getStorage(const Func &func) {
  Storage storage;
  updateStorage(func, &storage);
  return storage;
}

Storage getStorage(const Stmt &stmt) {
  Storage storage;
  updateStorage(stmt, &storage);
  return storage;
}

void updateStorage(const Func &func, Storage *storage) {
  GetStorageVisitor(storage).get(func);
}

void updateStorage(const Stmt &stmt, Storage *storage) {
  GetStorageVisitor(storage).get(stmt);
}

}}

#include "storage.h"

#include <memory>

#include "ir.h"
#include "ir_visitor.h"
#include "path_expressions.h"
#include "tensor_index.h"
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

  pe::PathExpression pathExpression;
  map<pair<unsigned,unsigned>, TensorIndex> tensorIndices;
};

TensorStorage::TensorStorage() : TensorStorage(Kind::Undefined) {
}

TensorStorage::TensorStorage(Kind kind) : content(new Content) {
  content->kind = kind;
}

TensorStorage::TensorStorage(const Expr &targetSet)
    : TensorStorage(Kind::SystemDiagonal) {
  content->systemTargetSet = targetSet;
}

TensorStorage::TensorStorage(const Expr &targetSet, const Expr &storageSet)
    : TensorStorage(Kind::SystemReduced) {
  content->systemTargetSet = targetSet;
  content->systemStorageSet = storageSet;
}

TensorStorage::Kind TensorStorage::getKind() const {
  return content->kind;
}

bool TensorStorage::isDense() const {
  return getKind() == Kind::DenseRowMajor;
}

bool TensorStorage::isSystem() const {
  switch (getKind()) {
    case Kind::DenseRowMajor:
      return false;
    case Kind::SystemReduced:
    case Kind::SystemDiagonal:
    case Kind::MatrixFree:
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
  string name = tensor.getName() + "_rows2cols";

  Var coordArray(name+"_coords", ArrayType::make(ScalarType::Int));
  Var sinkArray(name+"_sinks",   ArrayType::make(ScalarType::Int));

  content->tensorIndices.insert({{srcDim,sinkDim},
                                 TensorIndex(coordArray, sinkArray,
                                             srcDim, sinkDim)});

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
    case TensorStorage::Kind::DenseRowMajor:
      os << "Dense Row Major";
      break;
    case TensorStorage::Kind::MatrixFree:
      os << "Matrix-Free";
      break;
    case TensorStorage::Kind::SystemReduced:
      os << "System Reduced";
      break;
    case TensorStorage::Kind::SystemDiagonal:
      os << "System Diagonal";
      break;
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
    iassert(!hasStorage(var)) << "Variable" << var << "already has storage";
    add(var, other.getStorage(var));
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
  
  using IRVisitor::visit;

  void visit(const VarDecl *op) {
    Var var = op->var;
    Type type = var.getType();
    iassert(!storage->hasStorage(var)) << "Redeclaration of variable" << var;
    if (type.isTensor() && !isScalar(type)) {
      determineStorage(var);
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

  void visit(const Map *op) {
    for (auto &var : op->vars) {
      Type type = var.getType();
      if (type.isTensor() && !isScalar(type)) {
        // For now we'll store all assembled vectors as dense and other tensors
        // as system reduced
        TensorStorage tensorStorage;
        auto tensorType = type.toTensor();
        if (tensorType->order() == 1) {
          tensorStorage = TensorStorage(TensorStorage::Kind::DenseRowMajor);
        }
        else if (op->neighbors.defined()) {
          tensorStorage = TensorStorage(op->target, op->neighbors);
        }
        else {
          tensorStorage = TensorStorage(op->target);
        }
        iassert(tensorStorage.getKind() != TensorStorage::Kind::Undefined);
        storage->add(var, tensorStorage);
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
      tensorStorage = TensorStorage(TensorStorage::Kind::DenseRowMajor);
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
        {TensorStorage::Kind::DenseRowMajor,  4},
        {TensorStorage::Kind::SystemReduced,  3},
        {TensorStorage::Kind::SystemDiagonal, 2},
        {TensorStorage::Kind::MatrixFree,     1},
        {TensorStorage::Kind::Undefined,      0}
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
            case TensorStorage::Kind::DenseRowMajor:
            case TensorStorage::Kind::MatrixFree:
              tensorStorage = operandStorage.getKind();
              break;
            case TensorStorage::Kind::SystemReduced:
              tensorStorage =
                  TensorStorage(operandStorage.getSystemTargetSet(),
                                operandStorage.getSystemStorageSet());
              break;
            case TensorStorage::Kind::SystemDiagonal:
              tensorStorage =
                  TensorStorage(operandStorage.getSystemTargetSet());
              break;
            case TensorStorage::Kind::Undefined:
              unreachable;
              break;
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

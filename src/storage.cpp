#include "storage.h"

#include "ir.h"
#include "ir_visitor.h"
#include "path_expressions.h"

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

  /// Whether the tensor needs storage allocated at runtime.
  bool needsInitialization;

  /// A path expression describing
  pe::PathExpression pathExpression;

  Content(Kind kind) : kind(kind) {}
};

TensorStorage::TensorStorage() : TensorStorage(Undefined) {
}

TensorStorage::TensorStorage(Kind kind, bool needsInitialization)
    : content(new Content(kind)) {
  content->needsInitialization = needsInitialization;
}

TensorStorage::TensorStorage(const Expr &targetSet)
    : TensorStorage(SystemDiagonal) {
  content->systemTargetSet = targetSet;
}

TensorStorage::TensorStorage(const Expr &targetSet, const Expr &storageSet)
    : TensorStorage(SystemReduced) {
  content->systemTargetSet = targetSet;
  content->systemStorageSet = storageSet;
}

TensorStorage::Kind TensorStorage::getKind() const {
  return content->kind;
}

bool TensorStorage::isDense() const {
  return content->kind == DenseRowMajor;
}

bool TensorStorage::isSystem() const {
  switch (content->kind) {
    case DenseRowMajor:
      return false;
    case MatrixFree:
    case SystemReduced:
    case SystemDiagonal:
      return true;
    case Undefined:
      ierror;
  }
  return false;
}

bool TensorStorage::needsInitialization() const {
  return content->needsInitialization;
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
    case TensorStorage::Undefined:
      os << "Undefined";
      break;
    case TensorStorage::DenseRowMajor:
      os << "Dense Row Major";
      break;
    case TensorStorage::MatrixFree:
      os << "Matrix-Free";
      break;
    case TensorStorage::SystemReduced:
      os << "System Reduced";
      break;
    case TensorStorage::SystemDiagonal:
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

void Storage::add(const Var &tensor, TensorStorage tstorage) {
  content->storage[tensor] = tstorage;
}

void Storage::add(const Storage &other) {
  for (auto &var : other) {
    iassert(!hasStorage(var)) << "Variable" << var << "already has storage";
    add(var, other.get(var));
  }
}

bool Storage::hasStorage(const Var &tensor) const {
  return content->storage.find(tensor) != content->storage.end();
}

TensorStorage &Storage::get(const Var &tensor) {
  iassert(hasStorage(tensor))
      << " no storage specified for tensor " << util::quote(tensor);
  return content->storage.at(tensor);
}

const TensorStorage &Storage::get(const Var &tensor) const {
  return const_cast<Storage*>(this)->get(tensor);
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
    os << *it << " : " << storage.get(*it);
    ++it;
  }
  for (; it != end; ++it) {
    os << std::endl << *it << " : " << storage.get(*it);
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
        determineStorage(global.first, false);
      }
    }

    for (auto &arg : func.getArguments()) {
      if (arg.getType().isTensor()) {
        determineStorage(arg, false);
      }
    }

    for (auto &res : func.getResults()) {
      if (res.getType().isTensor()) {
        determineStorage(res, false);
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
      determineStorage(var, true);
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
        determineStorage(var, !isa<Literal>(op->value), op->value);
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
          tensorStorage = TensorStorage(TensorStorage::DenseRowMajor);
        }
        else if (op->neighbors.defined()) {
          tensorStorage = TensorStorage(op->target, op->neighbors);
        }
        else {
          tensorStorage = TensorStorage(op->target);
        }
        iassert(tensorStorage.getKind() != TensorStorage::Undefined);
        storage->add(var, tensorStorage);
      }
    }
  }

  void determineStorage(Var var, bool initialize=true, Expr rhs=Expr()) {
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
      tensorStorage = TensorStorage(TensorStorage::DenseRowMajor, initialize);
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
        {TensorStorage::DenseRowMajor,  4},
        {TensorStorage::SystemReduced,  3},
        {TensorStorage::SystemDiagonal, 2},
        {TensorStorage::MatrixFree,     1},
        {TensorStorage::Undefined,      0}
      };

      for (Var operand : leafVars.vars) {
        if (isScalar(operand.getType())) {
          continue;
        }

        iassert(storage->hasStorage(operand))
            << operand << "does not have a storage descriptor";

        const TensorStorage operandStorage  = storage->get(operand);
        auto operandStorageKind = operandStorage.getKind();
        auto tensorStorageKind = tensorStorage.getKind();
        if (priorities[operandStorageKind] > priorities[tensorStorageKind]) {
          switch (operandStorage.getKind()) {
            case TensorStorage::DenseRowMajor:
            case TensorStorage::MatrixFree:
              tensorStorage = operandStorage.getKind();
              break;
            case TensorStorage::SystemReduced:
              tensorStorage =
                  TensorStorage(operandStorage.getSystemTargetSet(),
                                operandStorage.getSystemStorageSet());
              break;
            case TensorStorage::SystemDiagonal:
              tensorStorage =
                  TensorStorage(operandStorage.getSystemTargetSet());
              break;
            case TensorStorage::Undefined:
              unreachable;
              break;
          }
        }
      }
    }

    if (tensorStorage.getKind() != TensorStorage::Undefined) {
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

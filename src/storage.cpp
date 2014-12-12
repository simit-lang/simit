#include "storage.h"

#include "ir.h"
#include "ir_visitor.h"

using namespace std;

namespace simit {
namespace ir {

// class TensorStorage
struct TensorStorage::Content {
  Kind kind;

  /// The target set that was used to assemble the system if the tensor is
  /// stored on a system, false otherwise.
  Expr systemStorageSet;

  Content(Kind kind) : kind(kind) {}
};

TensorStorage::TensorStorage() : TensorStorage(Undefined) {
}

TensorStorage::TensorStorage(Kind kind) : content(new Content(kind)) {
}

TensorStorage::Kind TensorStorage::getKind() const {
  return content->kind;
}

/// True if the tensor is stored on a system, false otherwise.
bool TensorStorage::isSystem() const {
  return content->kind==SystemNone || content->kind==SystemReduced ||
         content->kind==SystemUnreduced;
}

void TensorStorage::setSystemStorageSet(const Expr &systemStorageSet) {
  content->systemStorageSet = systemStorageSet;
}

const Expr &TensorStorage::getSystemTargetSet() const {
  iassert(!isSystem() || content->systemStorageSet.defined())
      << "System storages require the target set be provided";
  return content->systemStorageSet;
}

std::ostream &operator<<(std::ostream &os, const TensorStorage &ts) {
  os << "TensorStorage(";
  switch (ts.getKind()) {
    case TensorStorage::Undefined:
      os << "Undefined";
      break;
    case TensorStorage::DenseRowMajor:
      os << "Dense Row Major";
      break;
    case TensorStorage::SystemNone:
      os << "System None";
      break;
    case TensorStorage::SystemReduced:
      os << "System Reduced";
      break;
    case TensorStorage::SystemUnreduced:
      os << "System Unreduced";
      break;
  }
  return os << ")";
}


// class Storage
struct Storage::Content {
  std::map<Var,TensorStorage> storage;
};

Storage::Storage() : content(new Content) {
}

void Storage::add(const Var &tensor, TensorStorage tstorage) {
  content->storage.insert(std::pair<Var,TensorStorage>(tensor,tstorage));
}

bool Storage::hasStorage(const Var &tensor) const {
  return content->storage.find(tensor) != content->storage.end();
}

TensorStorage &Storage::get(const Var &tensor) {
  iassert(hasStorage(tensor)) << "no storage specified for tensor" << tensor;
  return content->storage.at(tensor);
}

const TensorStorage &Storage::get(const Var &tensor) const {
  return const_cast<Storage*>(this)->get(tensor);
}


// Free functions
class GetStorage : public IRVisitor {
public:
  Storage get(Func func) {
    for (auto &arg : func.getArguments())
      if (arg.getType().isTensor())
        determineStorage(arg);

    for (auto &res : func.getResults())
      if (res.getType().isTensor())
        determineStorage(res);

    func.accept(this);
    return storage;
  }

  Storage get(Stmt stmt) {
    stmt.accept(this);
    return storage;
  }

private:
  Storage storage;

  void visit(const AssignStmt *op) {
    Var var = op->var;
    Type type = var.getType();
    if (type.isTensor() && !isScalar(type) && !storage.hasStorage(var)) {
      determineStorage(var);
    }
  }

  void visit(const Map *op) {
    for (auto &var : op->vars) {
      Type type = var.getType();
      if (type.isTensor() && !isScalar(type)) {
        if (!storage.hasStorage(var)) {
          // For now we'll store all assembled tensors as system reduced
          storage.add(var, TensorStorage(TensorStorage::SystemReduced));
        }
      }
    }
  }

  TensorStorage determineStorage(Var var) {
    // If all dimensions are ranges then we choose dense row major. Otherwise,
    // we choose system reduced storage order (for now).
    Type type = var.getType();
    iassert(type.isTensor());
    const TensorType *ttype = type.toTensor();

    TensorStorage tensorStorage;
    if (isElementTensorType(ttype) || ttype->order() <= 1) {
      tensorStorage = TensorStorage(TensorStorage::DenseRowMajor);
    }
    else {
      tensorStorage = TensorStorage(TensorStorage::SystemReduced);
    }
    storage.add(var, tensorStorage);
    return tensorStorage;
  }
};

Storage getStorage(const Func &func) {
  return GetStorage().get(func);
}

Storage getStorage(const Stmt &stmt) {
  return GetStorage().get(stmt);
}

}}

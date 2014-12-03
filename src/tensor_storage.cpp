#include "tensor_storage.h"

#include "ir_visitor.h"

using namespace std;

namespace simit {
namespace ir {

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

class GetTensorStorages : public IRVisitor {
public:
  std::map<Var,TensorStorage> get(Func func) {
    for (auto &arg : func.getArguments())
      if (arg.getType().isTensor())
        determineStorage(arg);

    for (auto &res : func.getResults())
      if (res.getType().isTensor())
        determineStorage(res);

    func.accept(this);
    return storages;
  }

private:
  TensorStorages storages;

  void visit(const AssignStmt *op) {
    Var var = op->var;
    Type type = var.getType();
    if (type.isTensor() && !isScalar(type) &&
        storages.find(var) == storages.end()) {
      determineStorage(var);
    }
  }

  void visit(const Map *op) {
    for (auto &var : op->vars) {
      Type type = var.getType();
      if (type.isTensor() && !isScalar(type) &&
          storages.find(var) == storages.end()) {
        TensorStorage storage = determineStorage(var);
        if (storage.getKind() == TensorStorage::SystemNone ||
            storage.getKind() == TensorStorage::SystemUnreduced) {
          storage.setSystemStorageSet(op->target);
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

    TensorStorage storage;
    if (isElementTensorType(ttype)) {
      storage = TensorStorage(TensorStorage::DenseRowMajor);
    }
    else {
      storage = TensorStorage(TensorStorage::SystemReduced);
//    storage.setSystemStorageSet(<#simit::ir::Expr systemStorageSet#>)
    }

    storages.insert(std::pair<Var,TensorStorage>(var, storage));
    return storage;
  }
};

TensorStorages getTensorStorages(Func func) {
  std::map<Var,TensorStorage> descriptors = GetTensorStorages().get(func);
  return descriptors;
}

}}

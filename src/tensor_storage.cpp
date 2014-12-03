#include "tensor_storage.h"

#include "ir_visitor.h"

using namespace std;

namespace simit {
namespace ir {

std::ostream &operator<<(std::ostream &os, const TensorStorage &ts) {
  os << "TensorStorage(";
  switch (ts.getKind()) {
    case TensorStorage::DenseRowMajor:
      os << "Dense Row Major";
      break;
    case TensorStorage::SystemNone:
      os << "System None";
      break;
    case TensorStorage::SystemReduced:
      os << "System Reduced";
      break;
  }
  return os << ")";
}

static TensorStorage determineStorage(Var var) {
  // If all dimensions are ranges then we choose dense row major. Otherwise,
  // we choose system reduced storage order (for now).
  Type type = var.getType();
  iassert(type.isTensor());
  const TensorType *ttype = type.toTensor();


  if (isElementTensorType(ttype)) {
    return TensorStorage(TensorStorage::DenseRowMajor);
  }
  else {

    return TensorStorage(TensorStorage::SystemReduced);
  }
}

class GetTensorStorages : public IRVisitor {
public:
  std::map<Var,TensorStorage> get(Func func) {
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
      storages.insert(std::pair<Var,TensorStorage>(var, determineStorage(var)));
    }
  }

  void visit(const Map *op) {
    for (auto &var : op->vars) {
      Type type = var.getType();
      if (type.isTensor() && !isScalar(type) &&
          storages.find(var) == storages.end()) {
        storages.insert(pair<Var,TensorStorage>(var,determineStorage(var)));
      }
    }
  }
};

TensorStorages getTensorStorages(Func func) {
  std::map<Var,TensorStorage> descriptors = GetTensorStorages().get(func);
  return descriptors;
}

}}

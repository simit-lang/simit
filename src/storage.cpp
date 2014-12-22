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
  /// stored on a system, undefined otherwise.
  Expr systemTargeteSet;
  Expr systemStorageSet;

  Content(Kind kind) : kind(kind) {}
};

TensorStorage::TensorStorage() : TensorStorage(Undefined) {
}

TensorStorage::TensorStorage(Kind kind) : content(new Content(kind)) {
}

TensorStorage::TensorStorage(Kind kind, const Expr &targetSet,
                             const Expr &storageSet) : TensorStorage(kind) {
  iassert(kind==SystemReduced);
  content->systemTargeteSet = targetSet;
  content->systemStorageSet = storageSet;
}

TensorStorage::Kind TensorStorage::getKind() const {
  return content->kind;
}

bool TensorStorage::isSystem() const {
  return content->kind==SystemNone || content->kind==SystemReduced ||
         content->kind==SystemUnreduced;
}

const Expr &TensorStorage::getSystemTargetSet() const {
  iassert(isSystem()) << "System storages require the target set be provided";
  return content->systemTargeteSet;
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
  return os;
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

  void visit(const TensorWrite *op) {
    if (isa<VarExpr>(op->tensor)) {
      const Var &var = to<VarExpr>(op->tensor)->var;
      Type type = var.getType();
      if (type.isTensor() && !isScalar(type) && !storage.hasStorage(var)) {
        determineStorage(var);
      }
    }
  }

  void visit(const Map *op) {
    for (auto &var : op->vars) {
      Type type = var.getType();
      if (type.isTensor() && !isScalar(type) && !storage.hasStorage(var)) {
        // For now we'll store all assembled vectors as dense and other tensors
        // as system reduced
        TensorStorage tensorStorage;
        auto tensorType = type.toTensor();
        if (tensorType->order() == 1) {
          tensorStorage = TensorStorage(TensorStorage::DenseRowMajor);
        }
        else {
          tensorStorage = TensorStorage(TensorStorage::SystemReduced,
                                        op->target, op->neighbors);
        }
        iassert(tensorStorage.getKind() != TensorStorage::Undefined);
        storage.add(var, tensorStorage);
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

}}

#include "storage.h"

#include <memory>

#include "init.h"
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
  Kind        kind;
  TensorIndex index;
};

TensorStorage::TensorStorage() : TensorStorage(Kind::Undefined) {
}

TensorStorage::TensorStorage(Kind kind) : content(new Content) {
  content->kind = kind;
}

TensorStorage::TensorStorage(Kind kind, const TensorIndex& index)
    : content(new Content) {
  content->kind = kind;
  content->index = index;
}

TensorStorage::Kind TensorStorage::getKind() const {
  return content->kind;
}

bool TensorStorage::hasTensorIndex() const {
  return content->index.defined();
}

const TensorIndex& TensorStorage::getTensorIndex() const {
  iassert((content->index.getKind() == TensorIndex::PExpr &&
           getKind() == TensorStorage::Indexed) ||
          (content->index.getKind() == TensorIndex::Sten &&
           getKind() == TensorStorage::Stencil))
      << "Expected Indexed tensor, but was " << *this;
  iassert(content->index.defined());
  return content->index;
}

TensorIndex& TensorStorage::getTensorIndex() {
  iassert((content->index.getKind() == TensorIndex::PExpr &&
           getKind() == TensorStorage::Indexed) ||
          (content->index.getKind() == TensorIndex::Sten &&
           getKind() == TensorStorage::Stencil))
      << "Expected Indexed tensor, but was " << *this;
  iassert(content->index.defined());
  return content->index;
}

void TensorStorage::setTensorIndex(Var tensor) {
  content->index = TensorIndex(tensor.getName()+"_index", pe::PathExpression());
}

std::ostream &operator<<(std::ostream &os, const TensorStorage &ts) {
  switch (ts.getKind()) {
    case TensorStorage::Undefined:
      os << "Undefined";
      break;
    case TensorStorage::Dense:
      os << "Dense";
      break;
    case TensorStorage::Indexed:
      os << "Indexed";
      if (ts.hasTensorIndex()) {
        os << " (" << ts.getTensorIndex().getPathExpression() << ")";
      }
      break;
    case TensorStorage::Stencil:
      os << "Stencil";
      break;
    case TensorStorage::Diagonal:
      os << "Diagonal";
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
  GetStorageVisitor(Storage *storage, Environment* env)
      : storage{storage}, env{env} {}

  void get(Func func) {
    for (auto &global : func.getEnvironment().getConstants()) {
      if (global.first.getType().isTensor()) {
        storage->add(global.first, TensorStorage::Dense);
      }
    }

    for (auto &arg : func.getArguments()) {
      if (arg.getType().isTensor()) {
        storage->add(arg, TensorStorage::Dense);
      }
    }

    for (auto &res : func.getResults()) {
      if (res.getType().isTensor()) {
        storage->add(res, TensorStorage::Dense);
      }
    }

    func.accept(this);
  }

  void get(Stmt stmt) {
    stmt.accept(this);
  }

private:
  Storage* storage;
  Environment* env;
  PathExpressionBuilder peBuilder;

  TensorIndex getTensorIndex(const Var& var) {
    auto pexpr = peBuilder.getPathExpression(var);
    if (!env->hasTensorIndex(pexpr)) {
      env->addTensorIndex(pexpr, var);
    }
    return env->getTensorIndex(pexpr);
  }

  TensorIndex setStencilTensorIndex(const Var& var, std::string assemblyFunc,
                                    std::string targetVar) {
    iassert(!env->hasTensorIndex(var));
    StencilContent *content = new StencilContent;
    content->assemblyFunc = assemblyFunc;
    content->targetVar = targetVar;
    auto stencil = StencilLayout(content);
    env->addTensorIndex(stencil, var);
    return env->getTensorIndex(stencil);
  }

  using IRVisitor::visit;

  void visit(const VarDecl *op) {
    Var var = op->var;
    Type type = var.getType();
    if (type.isTensor() && !isScalar(type)) {
      storage->add(var, TensorStorage::Dense);
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
          storage->add(var, TensorStorage::Dense);
        }
      }
      // System matrices
      else {
        pe::PathExpression pexpr;
        if (isa<IndexExpr>(op->value)) {
          peBuilder.computePathExpression(var, to<IndexExpr>(op->value));
          pexpr = peBuilder.getPathExpression(var);
        }

        inferStorage(var, op->value);
      }
    }
  }

  void visit(const TensorWrite *op) {
    if (isa<VarExpr>(op->tensor)) {
      const Var &var = to<VarExpr>(op->tensor)->var;
      Type type = var.getType();
      if (type.isTensor() && !isScalar(type) && !storage->hasStorage(var)) {
        storage->add(var, TensorStorage::Dense);
      }
    }
  }

  void visit(const CallStmt* op) {
    // Classify the result of extern functions.
    if (op->callee.getKind() == Func::External) {
      for (auto& result : op->results) {
        if (result.getType().isTensor()) {
          auto type = result.getType().toTensor();
          if (type->order() == 1 || !type->hasSystemDimensions()) {
            storage->add(result, TensorStorage(TensorStorage::Dense));
          }
          else {
            // Sparse matrices returned from extern functions have undefined
            // path expressions (sparsity not known)
            auto index = TensorIndex(result.getName(), pe::PathExpression());
            storage->add(result, TensorStorage(TensorStorage::Indexed, index));
          }
        }
      }
    }
  }

  void visit(const Map *op) {
    // If the map target set is not an edge set, then matrices are dense,
    // stencil, or diagonal. Otherwise, the matrices are indexed with a path
    // expression
    Type targetType = op->target.type();
    iassert(targetType.isSet());
    if (targetType.isLatticeLinkSet() ||
        (targetType.isUnstructuredSet() &&
         targetType.toUnstructuredSet()->getCardinality() == 0)) {
      for (const Var& var : op->vars) {
        iassert(var.getType().isTensor());
        const TensorType* type = var.getType().toTensor();

        if (type->order() < 2) {
          // Scalars and vectors are dense
          storage->add(var, TensorStorage(TensorStorage::Dense));
        }
        else if (op->through.defined()) {
          if (kIndexlessStencils) {
            // Indexless Stencil
            auto index = setStencilTensorIndex(
                var, op->function.getName(), var.getName());
            storage->add(var, TensorStorage(TensorStorage::Stencil, index));
          }
          else {
            // Indexed Stencil
            peBuilder.computePathExpression(op);
            auto index = getTensorIndex(var);
            storage->add(var, TensorStorage(TensorStorage::Indexed, index));
          }
        }
        else {
          // Diagonal
          storage->add(var, TensorStorage(TensorStorage::Diagonal));
        }
      }
    }
    else {
      peBuilder.computePathExpression(op);

      for (const Var& var : op->vars) {
        Type type = var.getType();
        if (type.isTensor() && !isScalar(type)) {
          // For now we'll store all assembled vectors as dense and other
          // tensors as system reduced
          TensorStorage tensorStorage;
          const TensorType* tensorType = type.toTensor();
          if (tensorType->order() == 1) {
            tensorStorage = TensorStorage(TensorStorage::Dense);
          }
          else {
            if (op->neighbors.empty()) {
              tensorStorage = TensorStorage(TensorStorage::Diagonal);
            }
            else {
              auto index = getTensorIndex(var);
              tensorStorage = TensorStorage(TensorStorage::Indexed, index);

              // Add path expression
              tassert(tensorType->order() == 2)
                  << "tensor has order " << tensorType->order()
                  << ", while we only currently supports sparse matrices";
            }
          }
          iassert(tensorStorage.getKind() != TensorStorage::Undefined);
          storage->add(var, tensorStorage);
        }
      }
    }
  }

  void inferStorage(Var var, Expr rhs) {
    iassert(rhs.defined()) << "Cannot infer storage from an undefined expr";

    // Scalars don't need storage
    if (isScalar(var.getType())) return;

    Type type = var.getType();
    iassert(type.isTensor());
    const TensorType *ttype = type.toTensor();

    TensorStorage tensorStorage;

    // Element tensor and system vectors are dense.
    if (isElementTensorType(ttype) || ttype->order() == 1 || !rhs.defined()) {
      tensorStorage = TensorStorage(TensorStorage::Dense);
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

      // When creating a matrix by combining matrices, the new matrix gets the
      // storage with higher priority from the inputs.
      // E.g. if one of the input variables to the RHS expression is dense then
      // the output becomes dense.
      static map<TensorStorage::Kind, unsigned> priorities = {
        {TensorStorage::Dense,     3},
        {TensorStorage::Indexed,   2},
        {TensorStorage::Diagonal,  1},
        {TensorStorage::Undefined, 0}
      };

      // Start out at lowest priority (monotonically nondecreasing)
      tensorStorage = TensorStorage(TensorStorage::Undefined);

      for (Var operand : leafVars.vars) {
        if (isScalar(operand.getType())) {
          continue;
        }

        iassert(storage->hasStorage(operand))
            << operand << "does not have a storage descriptor";

        auto operandStorage  = storage->getStorage(operand);
        auto operandStorageKind = operandStorage.getKind();
        auto tensorStorageKind = tensorStorage.getKind();

        if (priorities[operandStorageKind] >= priorities[tensorStorageKind]) {
          switch (operandStorage.getKind()) {
            case TensorStorage::Dense:
              tensorStorage = TensorStorage(TensorStorage::Dense);
              break;
            case TensorStorage::Indexed: {
              auto operandIndex = operandStorage.getTensorIndex();
              TensorIndex index;
              if (tensorStorage.getKind() != TensorStorage::Indexed) {
                index = operandIndex.getPathExpression().defined()
                    ? getTensorIndex(var)
                    : TensorIndex(var.getName()+"_index", pe::PathExpression());
              }
              else if (!operandIndex.getPathExpression().defined()) {
                index=TensorIndex(var.getName()+"_index", pe::PathExpression());
              }
              if (index.defined()) {
                tensorStorage = TensorStorage(TensorStorage::Indexed, index);
              }
              break;
            }
            case TensorStorage::Stencil: {
              auto index = getTensorIndex(var);
              tensorStorage = TensorStorage(TensorStorage::Stencil, index);
              break;
            }
            case TensorStorage::Diagonal:
              tensorStorage = TensorStorage(TensorStorage::Diagonal);
              break;
            case TensorStorage::Undefined:
              unreachable;
              break;
            default:
              unreachable;
          }
        }
      }
    }

    if (tensorStorage.getKind() != TensorStorage::Undefined) {
      storage->add(var, tensorStorage);
    }
  }
};

void updateStorage(const Func& func, Storage* storage, Environment* env) {
  GetStorageVisitor(storage, env).get(func);
}

void updateStorage(const Stmt& stmt, Storage* storage, Environment* env) {
  GetStorageVisitor(storage, env).get(stmt);
}

}}

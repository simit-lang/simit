#include "ir_transforms.h"

#include <unordered_map>

#include "ir_queries.h"
#include "ir_rewriter.h"
#include "util/collections.h"

using namespace std;

namespace simit {
namespace ir {

Func insertVarDecls(Func func) {
  class InsertVarDeclsRewriter : public IRRewriter {
    /// The set of variables that have already been declared. We do not need a
    /// scoped map here because Vars are compared by identity and cannot shadow.
    set<Var> declared;

    void visit(const Func *f) {
      declared.clear();

      for (auto &argument : f->getArguments()) {
        declared.insert(argument);
      }

      for (auto &result : f->getResults()) {
        declared.insert(result);
      }

      for (auto &constant : f->getEnvironment().getConstants()) {
        declared.insert(constant.first);
      }

      for (auto &temp : f->getEnvironment().getTemporaries()) {
        declared.insert(temp);
      }

      IRRewriter::visit(f);
    }

    void visit(const VarDecl *op) {
      declared.insert(op->var);
      stmt = op;
    }

    void visit(const AssignStmt *op) {
      if (!util::contains(declared, op->var)) {
        stmt = Block::make(VarDecl::make(op->var), op);
        declared.insert(op->var);
      }
      else {
        stmt = op;
      }
    }

    void visit(const Map *op) {
      stmt = op;
      for (auto &var : op->vars) {
        if (!util::contains(declared, var)) {
          stmt = Block::make(VarDecl::make(var), stmt);
          declared.insert(var);
        }
      }
    }

    void visit(const CallStmt *op) {
      stmt = op;
        for (auto &var : op->results) {
          if (!util::contains(declared, var)) {
            stmt = Block::make(VarDecl::make(var), stmt);
            declared.insert(var);
          }
        }
    }
  };
  return InsertVarDeclsRewriter().rewrite(func);
}

std::pair<Stmt,std::vector<Stmt>> removeVarDecls(Stmt stmt) {
  class RemoveVarDeclsRewriter : public IRRewriter {
  public:
    std::vector<Stmt> varDecls;

    void visit(const VarDecl *op) {
      varDecls.push_back(op);
      stmt = Stmt();
    }
  };
  RemoveVarDeclsRewriter rewriter;

  Stmt result = rewriter.rewrite(stmt);
  return std::pair<Stmt,vector<Stmt>>(result, rewriter.varDecls);
}

Stmt moveVarDeclsToFront(Stmt stmt) {
  std::pair<Stmt,vector<Stmt>> varDecls = removeVarDecls(stmt);
  return (varDecls.second.size() > 0)
      ? Block::make(Block::make(varDecls.second), varDecls.first)
      : varDecls.first;
}

Func makeSystemTensorsGlobal(Func func) {
  class MakeSystemTensorsGlobalRewriter : public IRRewriter {
    Environment environment;

    void visit(const Func* f) {
      environment = f->getEnvironment();

      Stmt body = rewrite(f->getBody());
      if (body != f->getBody()) {
        func = Func(f->getName(), f->getArguments(), f->getResults(), body,
                    environment);
        func.setStorage(f->getStorage());
      }
      else {
        func = *f;
      }
    }

    void visit(const VarDecl* op) {
      if (isSystemTensorType(op->var.getType())) {
        environment.addTemporary(op->var);
      }
      else {
        stmt = op;
      }
    }
  };
  return MakeSystemTensorsGlobalRewriter().rewrite(func);
}

Func insertInitializations(Func func) {
  struct CollectVars : public IRVisitor {
    struct VarOrField : public pair<Var, string> {
      VarOrField() = default;
      VarOrField(Var var) : pair<Var, string>(var, "") {}
      VarOrField(Var var, string field) : pair<Var, string>(var, field) {}
    };

    struct HashVarOrField {
      size_t operator() (const VarOrField& x) const {
        return (size_t) x.first.ptr;
      }
    };

    unordered_map<VarOrField, bool, HashVarOrField> vars; // true = inited

    void merge(const unordered_map<VarOrField, bool, HashVarOrField> & other) {
      for (const auto& kvp : other) {
        auto it = vars.find(kvp.first);
        if (it != vars.end()) {
          vars[kvp.first] = it->second && kvp.second;
        } else {
          vars[kvp.first] = kvp.second;
        }
      }
    }

    virtual void visit(const Func* op) {
      for (Var arg : op->getArguments()) {
        vars.insert({arg, true});
      }
      IRVisitor::visit(op);
    }

    virtual void visit(const CallStmt* op) {
      IRVisitor::visit(op);
      for (Var result : op->results) {
        vars.insert({result, true});
      }
    }

    virtual void visit(const Map* op) {
      IRVisitor::visit(op);
      for (Var result : op->vars) {
        vars.insert({result, true});
      }
    }

    virtual void visit(const VarExpr* op) {
      vars.insert({op->var, false});
    }

    virtual void visit(const AssignStmt* op) {
      IRVisitor::visit(op);
      vars.insert({op->var, true});
    }

    virtual void visit(const TensorRead* op) {
      IRVisitor::visit(op);
      vars.insert({getMainTensor(op), false});
    }

    virtual void visit(const TensorWrite* op) {
      op->value.accept(this);
      for (Expr index : op->indices) {
        index.accept(this);
      }
      vars.insert({getMainTensor(op->tensor), true});
      op->tensor.accept(this);
    }

    virtual void visit(const FieldRead* op) {
      IRVisitor::visit(op);
      vars.insert({VarOrField(getMainTensor(op), op->fieldName), false});
    }

    virtual void visit(const FieldWrite* op) {
      op->value.accept(this);
      vars.insert({VarOrField(getMainTensor(op->elementOrSet),
                   op->fieldName), true});
      op->elementOrSet.accept(this);
    }

    virtual void visit(const Load* op) {
      IRVisitor::visit(op);
      vars.insert({getMainTensor(op), false});
    }

    virtual void visit(const Store* op) {
      op->value.accept(this);
      op->index.accept(this);
      vars.insert({getMainTensor(op->buffer), true});
      op->buffer.accept(this);
    }

    virtual void visit(const IfThenElse* op) {
      op->condition.accept(this);
      auto copyVars = vars;
      op->thenBody.accept(this);
      if (op->elseBody.defined()) {
        swap(vars, copyVars);
        op->elseBody.accept(this);
        merge(copyVars);
      }
    }

    virtual void visit(const For* op) {
      auto copyVars = vars;
      op->body.accept(this);
      merge(copyVars);
    }

    virtual void visit(const ForRange* op) {
      op->start.accept(this);
      op->end.accept(this);
      auto copyVars = vars;
      op->body.accept(this);
      merge(copyVars);
    }

    virtual void visit(const While* op) {
      auto copyVars = vars;
      op->condition.accept(this);
      op->body.accept(this);
      merge(copyVars);
    }
  } visitor;
  func.accept(&visitor);

  struct HashVar {
    size_t operator()(const Var& a) const {
      return (size_t) a.ptr;
    }
  };

  unordered_multimap<Var, Stmt, HashVar> initStmts;
  for (const auto& kvp : visitor.vars) {
    Var var = kvp.first.first;
    if (!var.defined()) {
      continue;
    }
    string field = kvp.first.second;
    if (!kvp.second) {
      const TensorType* type;
      Type::Kind kind = var.getType().kind();
      switch (kind) {
      case Type::Tensor:
        type = var.getType().toTensor();
        break;
      case Type::Element:
      case Type::Set: {
        const ElementType* elementType = (kind == Type::Set ?
            var.getType().toSet()->elementType.toElement() :
            var.getType().toElement());
        auto fieldIt = elementType->fieldNames.find(field);
        if (fieldIt != elementType->fieldNames.end()) {
          Type t = elementType->fields[fieldIt->second].type;
          if (t.isTensor()) {
            type = t.toTensor();
            break;
          }
        }
      }
      default: // unable to determine type, skip initialization
        continue;
      }

      Expr zero;
      switch (type->getComponentType().kind) {
      case ScalarType::Float:
        zero = Literal::make(0.0);
        break;
      case ScalarType::Int:
        zero = Literal::make(0);
        break;
      case ScalarType::Boolean:
        zero = Literal::make(false);
        break;
      case ScalarType::Complex:
        zero = Literal::make(double_complex(0, 0));
        break;
      default:
        continue;
      }

      if (type->order() == 0) {
        switch (kind) {
        case Type::Tensor:
          initStmts.insert({var, AssignStmt::make(var, zero)});
          break;
        case Type::Element:
          initStmts.insert({var, FieldWrite::make(var, field, zero)});
          break;
        case Type::Set:
          Var element("e", var.getType().toSet()->elementType);
          initStmts.insert({var, For::make(element, ForDomain(IndexSet(var)),
                            FieldWrite::make(element, field, zero))});
          break;
        }
      } else {
        vector<Expr> lengths;
        size_t length = 1;
        for (const auto& domain : type->getDimensions()) {
          for (const IndexSet& indexSet : domain.getIndexSets()) {
            if (indexSet.getKind() == IndexSet::Range) {
              length *= indexSet.getSize();
            } else {
              lengths.push_back(Length::make(indexSet));
            }
          }
        }
        if (length == 0) { // nothing to init
          continue;
        }
        if (length > 1 || lengths.size() == 0) {
          lengths.push_back(Literal::make((int) length));
        }
        Expr end = lengths[0];
        for (size_t i = 1; i < lengths.size(); i++) {
          end = Mul::make(end, lengths[i]);
        }

        Expr buffer;
        Var element;
        switch (kind) {
        case Type::Tensor:
          buffer = var;
          break;
        case Type::Element:
          buffer = FieldRead::make(var, field);
          break;
        case Type::Set:
          element = Var("e", var.getType().toSet()->elementType);
          buffer = FieldRead::make(element, field);
          break;
        }
        Var index("i", Int);
        Stmt setZero = Store::make(buffer, index, zero);
        setZero = ForRange::make(index, 0, end, setZero);
        if (kind == Type::Set) {
          setZero = For::make(element, ForDomain(IndexSet(var)), setZero);
        }
        initStmts.insert({var, setZero});
      }
    }
  }

  struct InsertInit : public IRRewriter {
    const unordered_multimap<Var, Stmt, HashVar>& initStmts;
    InsertInit(const unordered_multimap<Var, Stmt, HashVar>& initStmts) :
      initStmts(initStmts) {}

    virtual void visit(const VarDecl* op) {
      auto pair = initStmts.equal_range(op->var);
      if (pair.first != pair.second) {
        vector<Stmt> stmts;
        stmts.push_back(op);
        for (;pair.first != pair.second; pair.first++) {
          stmts.push_back(pair.first->second);
        }
        stmt = Block::make(stmts);
      } else {
        stmt = op;
      }
    }

  } insertInit(initStmts);
  func = insertInit.rewrite(func);
  return func;
}

}}

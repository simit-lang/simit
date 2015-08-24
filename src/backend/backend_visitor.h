#ifndef SIMIT_BACKEND_VISITOR_H
#define SIMIT_BACKEND_VISITOR_H

#include "ir_visitor.h"

namespace simit {
namespace backend {

class BackendVisitor : public ir::IRVisitor { // Change to private inheritance
protected:
  virtual void compile(const ir::Literal&) = 0;
  virtual void compile(const ir::VarExpr&) = 0;
  virtual void compile(const ir::Load&) = 0;
  virtual void compile(const ir::FieldRead&) = 0;
  virtual void compile(const ir::Call&) = 0;
  virtual void compile(const ir::Length&) = 0;
  virtual void compile(const ir::IndexRead&) = 0;
  virtual void compile(const ir::TensorIndexRead&) = 0;

  virtual void compile(const ir::Neg&) = 0;
  virtual void compile(const ir::Add&) = 0;
  virtual void compile(const ir::Sub&) = 0;
  virtual void compile(const ir::Mul&) = 0;
  virtual void compile(const ir::Div&) = 0;

  virtual void compile(const ir::Not&) = 0;
  virtual void compile(const ir::Eq&) = 0;
  virtual void compile(const ir::Ne&) = 0;
  virtual void compile(const ir::Gt&) = 0;
  virtual void compile(const ir::Lt&) = 0;
  virtual void compile(const ir::Ge&) = 0;
  virtual void compile(const ir::Le&) = 0;
  virtual void compile(const ir::And&) = 0;
  virtual void compile(const ir::Or&) = 0;
  virtual void compile(const ir::Xor&) = 0;

  virtual void compile(const ir::VarDecl&) = 0;
  virtual void compile(const ir::AssignStmt&) = 0;
  virtual void compile(const ir::CallStmt&) = 0;
  virtual void compile(const ir::Store&) = 0;
  virtual void compile(const ir::FieldWrite&) = 0;
  virtual void compile(const ir::Block&) = 0;
  virtual void compile(const ir::IfThenElse&) = 0;
  virtual void compile(const ir::ForRange&) = 0;
  virtual void compile(const ir::For&) = 0;
  virtual void compile(const ir::While&) = 0;
  virtual void compile(const ir::Print&) = 0;

  // Optional
  virtual void compile(const ir::Pass&) {}

private:
  void visit(const ir::Literal*);
  void visit(const ir::VarExpr*);
  void visit(const ir::Load*);
  void visit(const ir::FieldRead*);
  void visit(const ir::Call*);
  void visit(const ir::Length*);
  void visit(const ir::IndexRead*);
  void visit(const ir::TensorIndexRead*);

  void visit(const ir::Neg*);
  void visit(const ir::Add*);
  void visit(const ir::Sub*);
  void visit(const ir::Mul*);
  void visit(const ir::Div*);

  void visit(const ir::Not*);
  void visit(const ir::Eq*);
  void visit(const ir::Ne*);
  void visit(const ir::Gt*);
  void visit(const ir::Lt*);
  void visit(const ir::Ge*);
  void visit(const ir::Le*);
  void visit(const ir::And*);
  void visit(const ir::Or*);
  void visit(const ir::Xor*);

  void visit(const ir::VarDecl*);
  void visit(const ir::AssignStmt*);
  void visit(const ir::CallStmt*);
  void visit(const ir::Store*);
  void visit(const ir::FieldWrite*);
  void visit(const ir::Block*);
  void visit(const ir::IfThenElse*);
  void visit(const ir::ForRange*);
  void visit(const ir::For*);
  void visit(const ir::While*);
  void visit(const ir::Print*);

  void visit(const ir::Pass*);

  /// High-level IRNodes that should be lowered and never reach the backend
  void visit(const ir::TupleRead*);
  void visit(const ir::TensorRead*);
  void visit(const ir::TensorWrite*);
  void visit(const ir::IndexedTensor*);
  void visit(const ir::IndexExpr*);
  void visit(const ir::Map*);
};

}}
#endif

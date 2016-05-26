#ifndef SIMIT_BACKEND_VISITOR_H
#define SIMIT_BACKEND_VISITOR_H

#include "ir_visitor.h"

namespace simit {
#ifdef GPU
namespace ir {
struct GPUKernel;
}
#endif

namespace backend {

class BackendVisitorBase : protected ir::IRVisitorStrict {
public:
  void accept(const ir::Expr& expr);
  void accept(const ir::Stmt& stmt);

protected:
  void visitError(std::string type, const void* op);

  void compile(const ir::Kernel&);
#ifdef GPU
  void compile(const ir::GPUKernel&);
#endif
  void compile(const ir::Block&);
  void compile(const ir::Comment&);
};

template <typename T>
class BackendVisitorExprHelper : protected BackendVisitorBase {
public:
  virtual T compile(const ir::Expr &expr) {
    accept(expr);
    T tmp = val;
    val = T();
    return tmp;
  }

  virtual void compile(const ir::Stmt &stmt) {
    accept(stmt);
    val = T();
  }

protected:
  T val = T();
};

template <>
class BackendVisitorExprHelper<void> : protected BackendVisitorBase {
public:
  void compile() {}
};

template <typename BackendExprType=void>
class BackendVisitor : public BackendVisitorExprHelper<BackendExprType> {
protected:
  virtual ~BackendVisitor() {}

  using BackendVisitorExprHelper<BackendExprType>::compile;

  virtual void compile(const ir::Literal&) = 0;
  virtual void compile(const ir::VarExpr&) = 0;
  virtual void compile(const ir::Load&) = 0;
  virtual void compile(const ir::FieldRead&) = 0;
  virtual void compile(const ir::Call&) = 0;
  virtual void compile(const ir::Length&) = 0;
  virtual void compile(const ir::IndexRead&) = 0;

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
  virtual void compile(const ir::Scope&) = 0;
  virtual void compile(const ir::IfThenElse&) = 0;
  virtual void compile(const ir::ForRange&) = 0;
  virtual void compile(const ir::For&) = 0;
  virtual void compile(const ir::While&) = 0;
  virtual void compile(const ir::Print&) = 0;


  // Optional

  /// The default Kernel compilation converts the kernel to a For loop and calls
  /// compile on it.
  virtual void compile(const ir::Kernel& op) {BackendVisitorBase::compile(op);}
  // TODO: GPUKernel should be generalized to Kernel
#ifdef GPU
  virtual void compile(const ir::GPUKernel& op) {BackendVisitorBase::compile(op);}
#endif

  virtual void compile(const ir::Block& op) {BackendVisitorBase::compile(op);}

  /// The default Comment compilation calls compile on the commented statement.
  virtual void compile(const ir::Comment& op) {BackendVisitorBase::compile(op);}

  virtual void compile(const ir::Pass&) {}

private:
  using ir::IRVisitorStrict::visit;
  void visit(const ir::Literal* op)    {compile(*op);}
  void visit(const ir::VarExpr* op)    {compile(*op);}
  void visit(const ir::Load* op)       {compile(*op);}
  void visit(const ir::FieldRead* op)  {compile(*op);}
  void visit(const ir::Call* op)       {compile(*op);}
  void visit(const ir::Length* op)     {compile(*op);}
  void visit(const ir::IndexRead* op)  {compile(*op);}
  void visit(const ir::Neg* op)        {compile(*op);}
  void visit(const ir::Add* op)        {compile(*op);}
  void visit(const ir::Sub* op)        {compile(*op);}
  void visit(const ir::Mul* op)        {compile(*op);}
  void visit(const ir::Div* op)        {compile(*op);}
  void visit(const ir::Not* op)        {compile(*op);}
  void visit(const ir::Eq* op)         {compile(*op);}
  void visit(const ir::Ne* op)         {compile(*op);}
  void visit(const ir::Gt* op)         {compile(*op);}
  void visit(const ir::Lt* op)         {compile(*op);}
  void visit(const ir::Ge* op)         {compile(*op);}
  void visit(const ir::Le* op)         {compile(*op);}
  void visit(const ir::And* op)        {compile(*op);}
  void visit(const ir::Or* op)         {compile(*op);}
  void visit(const ir::Xor* op)        {compile(*op);}
  void visit(const ir::VarDecl* op)    {compile(*op);}
  void visit(const ir::AssignStmt* op) {compile(*op);}
  void visit(const ir::CallStmt* op)   {compile(*op);}
  void visit(const ir::Store* op)      {compile(*op);}
  void visit(const ir::FieldWrite* op) {compile(*op);}
  void visit(const ir::Scope* op)      {compile(*op);}
  void visit(const ir::IfThenElse* op) {compile(*op);}
  void visit(const ir::ForRange* op)   {compile(*op);}
  void visit(const ir::For* op)        {compile(*op);}
  void visit(const ir::While* op)      {compile(*op);}
  void visit(const ir::Kernel *op)     {compile(*op);}
  void visit(const ir::Block* op)      {compile(*op);}
  void visit(const ir::Print* op)      {compile(*op);}
  void visit(const ir::Comment* op)    {compile(*op);}
  void visit(const ir::Pass* op)       {compile(*op);}

#ifdef GPU
  void visit(const ir::GPUKernel* op)     {compile(*op);}
#endif

  /// High-level IRNodes that should be lowered and never reach the backend
  using BackendVisitorBase::visitError;
  void visit(const ir::IndexExpr* op)     {visitError("IndexExpr", op);}
  void visit(const ir::TensorRead* op)    {visitError("TensorRead", op);}
  void visit(const ir::TupleRead* op)     {visitError("TupleRead", op);}
  void visit(const ir::IndexedTensor* op) {visitError("IndexedTensor", op);}
  void visit(const ir::Map* op)           {visitError("Map", op);}
  void visit(const ir::TensorWrite* op)   {visitError("TensorWrite", op);}
  void visit(const ir::SetRead* op)       {visitError("SetRead", op);}
};

}}
#endif

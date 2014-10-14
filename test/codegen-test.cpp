#include "gtest/gtest.h"

#include <memory>

#include "ir.h"
#include "ir_printer.h"
#include "backend.h"
#include "llvm_backend.h"

using namespace std;
using namespace testing;
using namespace simit;
using namespace simit::ir;
using namespace simit::internal;

template <typename T>
std::vector<T> toVectorOf(Expr expr) {
  std::vector<T> vec;
  const Literal *lit = dynamic_cast<const Literal*>(expr.expr());
  assert(lit);

  assert(lit->type.isScalar() || lit->type.isTensor());
  size_t size = (lit->type.isScalar()) ? 1 : lit->type.toTensor()->size();

  T *vals = static_cast<T*>(lit->data);
  vec.assign(vals, vals + size);
  return vec;
}

TEST(Codegen, add0) {
  Expr a = Variable::make("a", Float(64));
  Expr b = Variable::make("b", Float(64));
  Expr c = Variable::make("c", Float(64));

  Expr axb = Add::make(a,b);
  Stmt body = AssignStmt::make({"c"}, axb);

  Func func = Func("add0", {a,b}, {c}, body);

  LLVMBackend backend;
  unique_ptr<Function> function(backend.compile(func));

  a = Literal::make(Float(64), {2.0});
  b = Literal::make(Float(64), {4.1});
  c = Literal::make(Float(64));

  function->bind("a", &a);
  function->bind("b", &b);
  function->bind("c", &c);

  function->run();

  vector<double> results = toVectorOf<double>(c);
  ASSERT_DOUBLE_EQ(results[0], 6.1);
}

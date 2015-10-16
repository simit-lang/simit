#include "simit-test.h"

#include <memory>
#include <cmath>

#include "tensor.h"
#include "ir.h"
#include "intrinsics.h"
#include "ir_printer.h"

using namespace std;
using namespace testing;
using namespace simit::ir;
using namespace simit::backend;

TEST(Codegen, add) {
  Var a("a", Float);
  Var b("b", Float);
  Var c("c", Float);

  Expr axb = Add::make(a,b);
  Stmt body = AssignStmt::make(c, axb);

  simit::ir::Environment env;
  env.addExtern(a);
  env.addExtern(b);
  env.addExtern(c);

  simit::Function function = getTestBackend()->compile(body, env);

  simit_float aArg = 2.0;
  simit_float bArg = 4.1;
  simit_float cRes = 0.0;

  function.bind("a", &aArg);
  function.bind("b", &bArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(6.1, cRes);
}

TEST(Codegen, sin) {
  Var a("a", Float);
  Var c("c", Float);

  Expr sin_a = Call::make(intrinsics::sin(), {a});
  Stmt body = AssignStmt::make(c, sin_a);

  Func func = Func("testsin", {a}, {c}, body);

  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(func);

  simit_float aArg = 2.0;
  simit_float cRes = 0.0;

  function.bind("a", &aArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(sin(2.0), cRes);
}

TEST(Codegen, cos) {
  Var a("a", Float);
  Var c("c", Float);

  Expr cos_a = Call::make(intrinsics::cos(), {a});
  Stmt body = AssignStmt::make(c, cos_a);

  Func func = Func("testcos", {a}, {c}, body);

  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(func);

  simit_float aArg = 2.0;
  simit_float cRes = 0.0;

  function.bind("a", &aArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(cos(2.0), cRes);
}

TEST(Codegen, sqrt) {
  Var a("a", Float);
  Var c("c", Float);

  Expr sqrt_a = Call::make(intrinsics::sqrt(), {a});
  Stmt body = AssignStmt::make(c, sqrt_a);

  Func func = Func("testsqrt", {a}, {c}, body);

  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(func);

  simit_float aVar = 5.0;
  simit_float cRes = 0.0;

  function.bind("a", &aVar);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(sqrt(5.0), cRes);
}

TEST(Codegen, log) {
  Var a("a", Float);
  Var c("c", Float);

  Expr log_a = Call::make(intrinsics::log(), {a});
  Stmt body = AssignStmt::make(c, log_a);

  Func func = Func("testlog", {a}, {c}, body);

  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(func);

  simit_float aArg = 5.0;
  simit_float cRes = 0.0;

  function.bind("a", &aArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(log(5.0), cRes);
}

TEST(Codegen, exp) {
  Var a("a", Float);
  Var c("c", Float);

  Expr exp_a = Call::make(intrinsics::exp(), {a});
  Stmt body = AssignStmt::make(c, exp_a);

  Func func = Func("testexp", {a}, {c}, body);

  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(func);

  simit_float aArg = 5.0;
  simit_float cRes = 0.0;

  function.bind("a", &aArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(exp(5.0), cRes);
}

TEST(Codegen, atan2) {
  Var a("a", Float);
  Var b("b", Float);
  Var c("c", Float);

  Expr atan2_ab = Call::make(intrinsics::atan2(), {a,b});
  Stmt body = AssignStmt::make(c, atan2_ab);

  Func func = Func("testatan2", {a,b}, {c}, body);

  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(func);

  simit_float aArg = 1.0;
  simit_float bArg = 2.0;
  simit_float cRes = 0.0;

  function.bind("a", &aArg);
  function.bind("b", &bArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(atan2(1.0,2.0), cRes);
}

TEST(Codegen, forloop) {
  Var i("i", Int);
  Var out("out", Int);
  Expr start = Expr(1);
  Expr end = Expr(4);
  Stmt body = AssignStmt::make(out, i, CompoundOperator::Add);
  Stmt loop = ForRange::make(i, start, end, body);

  Func func = Func("testloop", {}, {out}, loop);

  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(func);

  int outRes = 0;

  function.bind("out", &outRes);

  function.runSafe();
  
  ASSERT_EQ(6, outRes);
}





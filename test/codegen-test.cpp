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

TEST(Codegen, add0) {
  Expr a = Variable::make("a", Float(64));
  Expr b = Variable::make("b", Float(64));
  Expr c = Variable::make("c", Float(64));

  Expr axb = Add::make(a,b);
  Stmt body = AssignStmt::make({"c"}, axb);

  Func func = Func("test", {a,b}, {c}, body);

  LLVMBackend backend;
  unique_ptr<Function> function(backend.compile(func));

  // Bind and run
}

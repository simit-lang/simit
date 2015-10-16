#include "simit-test.h"

#include "tensor.h"
#include "ir.h"
#include "intrinsics.h"
#include "ir_printer.h"
#include "graph.h"
#include "program.h"
#include "error.h"
#include "types.h"


using namespace std;
using namespace testing;
using namespace simit::ir;
using namespace simit::backend;

extern "C" void test_ext_func(simit_float a, simit_float b, simit_float *c) {
  *c = a+b;
}

TEST(ffi, extern_func) {
  Var a("a", Float);
  Var b("b", Float);
  Var c("c", Float);
  
  // this test is akin to declaring an external func called "test_ext_func"
  // and then creating another func "tst_func" that calls the external func.
  
  Func ext_func = Func("test_ext_func", {a, b}, {c}, Func::External);
  Stmt call_to_ext_func = CallStmt::make({c}, ext_func, {a,b});
  //Stmt body = AssignStmt::make(c, call_to_ext_func);
  Func tst_func = Func("test_extern_func", {a,b}, {c}, call_to_ext_func);
  
  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(tst_func);
  
  
  simit_float aArg = 1.0;
  simit_float bArg = 4.0;
  simit_float cRes = -1.0;

  function.bind("a", &aArg);
  function.bind("b", &bArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(1.0+4.0, cRes);
}

extern "C" void ext_vec3f_add(simit_float *a, simit_float *b, simit_float *c) {
  c[0] = a[0] + b[0];
  c[1] = a[1] + b[1];
  c[2] = a[2] + b[2];
}

TEST(ffi, vector_add_using_extern) {
  simit::Set points;
  simit::FieldRef<simit_float> a = points.addField<simit_float>("a");
  simit::FieldRef<simit_float> b = points.addField<simit_float>("b");
  simit::FieldRef<simit_float> c = points.addField<simit_float>("c");

  simit::ElementRef p0 = points.add();
  a.set(p0, 42.0);
  b.set(p0, 24.0);
  c.set(p0, -1.0);
  
  simit::ElementRef p1 = points.add();
  a.set(p1, 20.0);
  b.set(p1, 14.0);
  c.set(p1, -1.0);
  
  simit::ElementRef p2 = points.add();
  a.set(p2, 12.0);
  b.set(p2, 21.0);
  c.set(p2, -1.0);
  
  
  simit::Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(66.0, (int)c.get(p0));
  SIMIT_EXPECT_FLOAT_EQ(34.0, (int)c.get(p1));
  SIMIT_EXPECT_FLOAT_EQ(33.0, (int)c.get(p2));

}

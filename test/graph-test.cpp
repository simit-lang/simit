#include "gtest/gtest.h"

#include <vector>

#include "graph.h"

using namespace std;
using namespace simit::internal;
using namespace simit;

TEST(FieldTests, AddAndGet) {
  Field foo(Type::INT);
  ASSERT_EQ(foo.size(), (unsigned int) 0);
  foo.add<int>(40);
  ASSERT_EQ(foo.size(), (unsigned int)1);
  
  int ret;
  foo.get(foo.size()-1, &ret);
  ASSERT_EQ(ret, 40);
}

TEST(FieldTests, Remove) {
  Field foo(Type::INT);
  
  foo.add(10);
  foo.add(20);
  foo.add(30);
  
  foo.remove(1);
  
  int ret;
  foo.get(1, &ret);
  ASSERT_EQ(foo.size(), 2);
  ASSERT_EQ(ret, 30);
}

TEST(FieldTests, Expansion) {
  Field foo(Type::FLOAT);
  
  for (int i=0; i<1028; i++)
    foo.add(1.05 + i);
  
  ASSERT_GT(foo.size(), 1025);
  
  double ret;
  foo.get(1025, &ret);
  ASSERT_EQ(ret, 1026.05);
}
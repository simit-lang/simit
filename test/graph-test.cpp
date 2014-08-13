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
  ASSERT_EQ(foo.size(), (unsigned int)2);
  ASSERT_EQ(ret, 30);
}

TEST(FieldTests, Expansion) {
  Field foo(Type::FLOAT);
  
  for (int i=0; i<1028; i++)
    foo.add(1.05 + i);
  
  ASSERT_GT(foo.size(), (unsigned int)1025);
  
  double ret;
  foo.get(1025, &ret);
  ASSERT_EQ(ret, 1026.05);
}

//// Set tests

TEST(SetTests, Utils) {
  ASSERT_EQ(type_of<int>(), Type::INT);
  ASSERT_EQ(type_of<double>(), Type::FLOAT);
}

TEST(SetTests, AddAndGetFromTwoFields) {
  Set myset;
  
  myset.addField(Type::INT);
  myset.addField(Type::FLOAT);
  
  ASSERT_EQ(myset.numFields(), (unsigned int)2);
  ASSERT_EQ(myset.size(), (unsigned int)0);
  
  unsigned int i = myset.addItem();
  myset.set(i, 0, 10);
  myset.set(i, 1, 101.1);
  
  ASSERT_EQ(myset.size(), 1);
  
  double ret;
  myset.get(0, 1, &ret);
  ASSERT_EQ(ret, 101.1);
  
}
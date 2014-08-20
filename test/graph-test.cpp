#include "gtest/gtest.h"

#include <vector>

#include "graph.h"

using namespace std;
using namespace simit::internal;
using namespace simit;

TEST(FieldTests, AddAndGet) {
  Field foo(Type::INT);
  ASSERT_EQ(foo.size(),  0);
  foo.add<int>(40);
  ASSERT_EQ(foo.size(), 1);
  
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

//// Set tests

TEST(SetTests, Utils) {
  ASSERT_EQ(type_of<int>(), Type::INT);
  ASSERT_EQ(type_of<double>(), Type::FLOAT);
}

TEST(SetTests, AddAndGetFromTwoFields) {
  Set myset;
  
  FieldHandle f1 = myset.addField(Type::INT);
  FieldHandle f2 = myset.addField(Type::FLOAT);
  
  ASSERT_EQ(myset.numFields(), 2);
  ASSERT_EQ(myset.size(), 0);
  
  ElementHandle i = myset.addItem();
  myset.set(i, f1, 10);
  myset.set(i, f2, 101.1);
  
  ASSERT_EQ(myset.size(), 1);
  
  double ret;
  myset.get(i, f2, &ret);
  ASSERT_EQ(ret, 101.1);
  
}

//// Iterator tests
TEST(ElementIteratorTests, TestElementIteratorLoop) {
  Set myset;
  
  FieldHandle f1 = myset.addField(Type::INT);
  FieldHandle f2 = myset.addField(Type::FLOAT);
  
  ASSERT_EQ(myset.numFields(), 2);
  ASSERT_EQ(myset.size(), 0);
  
  for (int i=0; i<10; i++) {
    auto el = myset.addItem();
    myset.set(el, f1, 5+i);
    myset.set(el, f2, 10.0+(double)i);
  }
  
  int howmany=0;
  for (Set::ElementIterator it=myset.begin(); it<myset.end(); it++) {
    auto el = *it;
    int val;
    myset.get(el, f1, &val);
    ASSERT_TRUE((val>=5) && (val<15));
    howmany++;
    cout << howmany <<endl;
  }
  ASSERT_EQ(howmany, 10);
  
  howmany=0;
  for (auto it : myset) {
    auto el = it;
    int val;
    myset.get(el, f1, &val);
    ASSERT_TRUE((val>=5) && (val<15));
    howmany++;
    cout << howmany <<endl;
  }
  ASSERT_EQ(howmany, 10);
}

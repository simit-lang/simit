#include "simit-test.h"

#include <vector>

#include "graph.h"

using namespace std;
using namespace simit;

//// Set tests

TEST(SetTests, Utils) {
  ASSERT_EQ(typeOf<int>(), ComponentType::INT);
  ASSERT_EQ(typeOf<simit_float>(), ComponentType::FLOAT);
}

TEST(Set, AddAndGetFromTwoFields) {
  Set<> myset;
  
  FieldRef<int> f1 = myset.addField<int>("intfld");
  FieldRef<simit_float> f2 = myset.addField<simit_float>("floatfld");
  
  ASSERT_EQ(myset.getSize(), 0);
  
  ElementRef i = myset.add();
  f1.set(i, 10);
  f2.set(i, 101.1);

  ASSERT_EQ(myset.getSize(), 1);
  
  ASSERT_EQ(10, f1.get(i));
  ASSERT_SIMIT_FLOAT_EQ(101.1, f2.get(i));
}

TEST(Set, IncreaseCapacity) {
  Set<> myset;
  
  auto fld = myset.addField<int>("foo");
  
  for (int i=0; i<1029; i++) {
    ElementRef item = myset.add();
    fld.set(item, i);
  }

  int count = 0;
  bool foundIt[1029];
  for (auto&& b : foundIt) {
    b = false;
  }
  
  for (auto it : myset) {
    int val = fld.get(it);
    foundIt[val] = true;
    count++;
  }
  
  for (int i=0; i<1029; i++)
    ASSERT_TRUE(foundIt[i]);
  ASSERT_EQ(count, 1029);
}

TEST(Set, FieldAccesByName) {
  Set<> myset;
  
  auto f1 = myset.addField<simit_float>("fltfld");
  auto f2 = myset.addField<int>("intfld");

  ElementRef i = myset.add();
  f1.set(i, 42.0);
  f2.set(i, 10);

  ASSERT_EQ(f1.get(i), myset.getField<simit_float>("fltfld").get(i));
  ASSERT_EQ(f2.get(i), myset.getField<int>("intfld").get(i));
}

// Iterator tests
TEST(ElementIteratorTests, TestElementIteratorLoop) {
  Set<> myset;
  
  FieldRef<int> f1 = myset.addField<int>("intfld");
  FieldRef<simit_float> f2 = myset.addField<simit_float>("floatfld");

  ASSERT_EQ(myset.getSize(), 0);
  
  for (int i=0; i<10; i++) {
    auto el = myset.add();
    f1.set(el, 5+i);
    f2.set(el, 10.0+(simit_float)i);
  }
  
  int howmany=0;
  for (Set<>::ElementIterator it=myset.begin(); it<myset.end(); it++) {
    auto el = *it;
    int val = f1.get(el);
    ASSERT_TRUE((val>=5) && (val<15));
    howmany++;
  }
  ASSERT_EQ(howmany, 10);
  
  howmany=0;
  for (auto it : myset) {
    auto el = it;
    int val = f1.get(el);
    ASSERT_TRUE((val>=5) && (val<15));
    howmany++;
  }
  ASSERT_EQ(howmany, 10);
}

TEST(Field, Scalar) {
  Set<> points;
  FieldRef<simit_float> x = points.addField<simit_float>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();

  TensorRef<simit_float> scalar1 = x.get(p0);
  ASSERT_SIMIT_FLOAT_EQ(0u, scalar1.getOrder());
  scalar1 = 1.1;

  x.set(p1, 4.4);

  simit_float scalar3 = x.get(p0);
  ASSERT_SIMIT_FLOAT_EQ(1.1, scalar3);

//  TensorRef<simit_float,3> vec2 = p1.get(x);
  TensorRef<simit_float> scalar4 = x.get(p1);
  ASSERT_SIMIT_FLOAT_EQ(4.4, scalar4);
}

TEST(Field, Vector) {
  Set<> points;
  FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();

  TensorRef<simit_float,3> vec1 = x.get(p0);
  ASSERT_SIMIT_FLOAT_EQ(1u, vec1.getOrder());
  vec1(0) = 1.1;
  vec1(1) = 2.2;
  vec1(2) = 3.3;

  x.set(p1, {4.4, 5.5, 6.6});

  TensorRef<simit_float,3> vec3 = x.get(p0);
  ASSERT_SIMIT_FLOAT_EQ(1.1, vec3(0));
  ASSERT_SIMIT_FLOAT_EQ(2.2, vec3(1));
  ASSERT_SIMIT_FLOAT_EQ(3.3, vec3(2));

  TensorRef<simit_float,3> vec4 = x.get(p1);
  ASSERT_SIMIT_FLOAT_EQ(4.4, vec4(0));
  ASSERT_SIMIT_FLOAT_EQ(5.5, vec4(1));
  ASSERT_SIMIT_FLOAT_EQ(6.6, vec4(2));
}

TEST(Field, Matrix) {
  Set<> points;
  FieldRef<simit_float,3,2> x = points.addField<simit_float,3,2>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();

  x.set(p0, {0.0, 1.1, 2.2, 0.0, 0.0, 3.3});

  TensorRef<simit_float,3,2> mat2 = x.get(p1);
  mat2(0,0) = 4.4;
  mat2(1,1) = 5.5;
  mat2(1,0) = 6.6;

  TensorRef<simit_float,3,2> mat3 = x.get(p0);
  ASSERT_SIMIT_FLOAT_EQ(1.1, mat3(0,1));
  ASSERT_SIMIT_FLOAT_EQ(2.2, mat3(1,0));
  ASSERT_SIMIT_FLOAT_EQ(3.3, mat3(2,1));

  TensorRef<simit_float,3,2> mat4 = x.get(p1);
  ASSERT_SIMIT_FLOAT_EQ(4.4, mat4(0,0));
  ASSERT_SIMIT_FLOAT_EQ(5.5, mat4(1,1));
  ASSERT_SIMIT_FLOAT_EQ(6.6, mat4(1,0));
}

TEST(Field, Tensor) {
  Set<> points;
  FieldRef<simit_float,2,3,4> x = points.addField<simit_float,2,3,4>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();

  TensorRef<simit_float,2,3,4> t1 = x.get(p0);
  ASSERT_SIMIT_FLOAT_EQ(3u, t1.getOrder());
  t1(0,1,3) = 1.1;
  t1(1,0,2) = 2.2;
  t1(2,1,0) = 3.3;

  TensorRef<simit_float,2,3,4> t2 = x.get(p1);
  t2(0,0,1) = 4.4;
  t2(1,1,3) = 5.5;
  t2(1,0,0) = 6.6;

  TensorRef<simit_float,2,3,4> t3 = x.get(p0);
  ASSERT_SIMIT_FLOAT_EQ(1.1, t3(0,1,3));
  ASSERT_SIMIT_FLOAT_EQ(2.2, t3(1,0,2));
  ASSERT_SIMIT_FLOAT_EQ(3.3, t3(2,1,0));

  TensorRef<simit_float,2,3,4> t4 = x.get(p1);
  ASSERT_SIMIT_FLOAT_EQ(4.4, t4(0,0,1));
  ASSERT_SIMIT_FLOAT_EQ(5.5, t4(1,1,3));
  ASSERT_SIMIT_FLOAT_EQ(6.6, t4(1,0,0));
}

TEST(EdgeSet, CreateAndGetEdge) {
  Set<> points;

  FieldRef<simit_float> x = points.addField<simit_float>("x");
  
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  
  TensorRef<simit_float> scalar1 = x.get(p0);
  scalar1 = 1.1;
  
  TensorRef<simit_float> scalar2 = x.get(p1);
  scalar2 = 3.1;

  Set<2> edges(points, points);
  FieldRef<int> y = edges.addField<int>("y");
  
  ElementRef e = edges.add(p0, p1);
  TensorRef<int> escalar = y.get(e);
  escalar = 54;
  
  ASSERT_SIMIT_FLOAT_EQ(x.get(edges.getEndpoint(e,0)), 1.1);
  ASSERT_SIMIT_FLOAT_EQ(x.get(edges.getEndpoint(e,1)), 3.1);
  ASSERT_EQ(y.get(e), 54);
}

//TEST(EdgeSet, ConstructorTest) {
//  Set<> points;
//  points.add();
//  auto p0 = points.add();
//  
//  Set<1> points2(points);
//  auto p1 = points2.add(p0);
//  
//  Set<2> edges(points, points2);
//  edges.add(p0, p1);
//  ASSERT_DEATH(edges.add(p1, p0), "Assertion.*" );
//}

TEST(EdgeSet, EdgeIteratorTest) {
  Set<> points;
  
  FieldRef<simit_float> x = points.addField<simit_float>("x");
  
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();
  ElementRef p3 = points.add();
  
  TensorRef<simit_float> scalar1 = x.get(p0);
  scalar1 = 1.1;
  
  TensorRef<simit_float> scalar2 = x.get(p1);
  scalar2 = 3.1;
  
  Set<4> edges(points, points, points, points);

  ElementRef e1 = edges.add(p0, p1, p3, p2);
  
   int count=0;
  for (auto iter=edges.endpoints_begin(e1);
       iter < edges.endpoints_end(e1);
       iter++) {
    if (count==0)
      ASSERT_SIMIT_FLOAT_EQ(x.get(*iter), 1.1);
    if (count==1)
      ASSERT_SIMIT_FLOAT_EQ(x.get(*iter), 3.1);
    count++;
  }
  
  ASSERT_EQ(count, 4);
}

TEST(GraphGenerator, createBox) {
  Set<> points;
  Set<2> edges(points, points);
  simit::FieldRef<simit_float,3> X = points.addField<simit_float,3>("x");

  Box box = createBox(&points, &edges, 3, 3, 3);

  for(unsigned x = 0; x < box.numX()-1; ++x) {
    for(unsigned y = 0; y < box.numY(); ++y) {
      for(unsigned z = 0; z < box.numZ(); ++z) {
        ASSERT_TRUE(box.getEdge(box(x,y,z), box(x+1,y,z)).defined());
      }
    }
  }

  // y edges
  for(unsigned x = 0; x < box.numX(); ++x) {
    for(unsigned y = 0; y < box.numY()-1; ++y) {
      for(unsigned z = 0; z < box.numZ(); ++z) {
        ASSERT_TRUE(box.getEdge(box(x,y,z), box(x,y+1,z)).defined());
      }
    }
  }

  // z edges
  for(unsigned x = 0; x < box.numX(); ++x) {
    for(unsigned y = 0; y < box.numY(); ++y) {
      for(unsigned z = 0; z < box.numZ()-1; ++z) {
        ASSERT_TRUE(box.getEdge(box(x,y,z), box(x,y,z+1)).defined());
      }
    }
  }

  ASSERT_EQ(box.getEdges().size(), 54u);
}

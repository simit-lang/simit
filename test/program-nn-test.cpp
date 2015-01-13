#include "simit-test.h"

#include <cmath>
#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(Program, DISABLED_nn) {
  // Points
  Set<> nodes;
  FieldRef<simit_float> outv = nodes.addField<simit_float>("outv");
  FieldRef<simit_float> inv = nodes.addField<simit_float>("inv");
  FieldRef<simit_float> d = nodes.addField<simit_float>("d");
  FieldRef<simit_float> print = nodes.addField<simit_float>("print");

  // Springs
  Set<2> edges(nodes,nodes);
  FieldRef<simit_float> w = edges.addField<simit_float>("w");

  int l0 = 3;
  int l1 = 3;

  //create nodes
  vector<simit::ElementRef> nodeRefs;

  //input layer
  for(int ii = 0; ii<l0; ii++){
    nodeRefs.push_back(nodes.add());
  }
  inv.set(nodeRefs[0], 0.1);
  inv.set(nodeRefs[1], 0.421512737531634);
  inv.set(nodeRefs[2], 0.68278294534791);


  //input bias node
  simit::ElementRef n = nodes.add();
  nodeRefs.push_back(n);
  inv.set(n, 1.0);

  //middle layer
  vector<simit::ElementRef> middleLayer;
  int nInput = l0+1;
  for(int ii = 0; ii<l1; ii++){
    nodeRefs.push_back(nodes.add());
  }

  //edges
  vector<simit::ElementRef> edgeRefs;
  for(int ii = 0; ii<l1; ii++){
    for(int jj = 0; jj<4; jj++){
      int inputIdx = ii+jj;
      if(inputIdx>=nInput){
        break;
      }
      int outputIdx = ii + nInput;
      simit::ElementRef edge = edges.add(nodeRefs[inputIdx], nodeRefs[outputIdx]);
      edgeRefs.push_back(edge);
      //has to initialize with some random weights
      simit_float rw = (ii*4.0+jj)/(4.0*l1)+0.01;
      w.set(edge, rw);
    }
  }

  std::cout << "Inv" << std::endl;
  for (auto &n : nodeRefs) {
    std::cout << (simit_float)inv.get(n) << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Weights" << std::endl;
  for (auto &e : edgeRefs) {
    std::cout << (simit_float)w.get(e) << std::endl;
  }
  std::cout << std::endl;

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("nodes", &nodes);
  f->bind("edges", &edges);

  f->runSafe();

  std::cout << "Check" << std::endl;
  for (auto &n : nodeRefs) {
    std::cout << (simit_float)print.get(n) << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Outv" << std::endl;
  for (auto &n : nodeRefs) {
    std::cout << (simit_float)outv.get(n) << std::endl;
  }

  // Check outputs
//  TensorRef<simit_float,3> f0 = f.get(p0);
//  ASSERT_SIMIT_FLOAT_EQ(2422.649730810374,  f0(0));
//  ASSERT_SIMIT_FLOAT_EQ(2422.649730810374,  f0(1));
//  ASSERT_SIMIT_FLOAT_EQ(2407.9347308103738, f0(2));
//
//  TensorRef<simit_float,3> f1 = f.get(p1);
//  ASSERT_SIMIT_FLOAT_EQ(-1961.3248654051868, f1(0));
//  ASSERT_SIMIT_FLOAT_EQ(-1961.3248654051868, f1(1));
//  ASSERT_SIMIT_FLOAT_EQ(-2029.9948654051868, f1(2));
//
//  TensorRef<simit_float,3> f2 = f.get(p2);
//  ASSERT_SIMIT_FLOAT_EQ(-461.3248654051871, f2(0));
//  ASSERT_SIMIT_FLOAT_EQ(-461.3248654051871, f2(1));
//  ASSERT_SIMIT_FLOAT_EQ(-480.9448654051871, f2(2));
}

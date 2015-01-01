#include "gtest/gtest.h"

#include <cmath>
#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(Program, DISABLED_nn) {
  Program program;
  std::string programText = R"(
element Node
  inv : float;
  d  : float;
  outv  : float;

  print : float;
end

element Edge
  w : float;
end

extern nodes : set{Node};
extern edges : set{Edge}(nodes, nodes);

func eval(n:Node) -> (v : tensor[nodes](float))
  v(n) = 1.0/(1.0+exp(-n.outv));
end

func deriv(n:Node) -> (d : tensor[nodes](float))
  d(n) = n.outv * (1.0-n.outv);
end

func wMat(e:Edge, n:(Node*2)) -> (W:tensor[nodes, nodes](float))
  W(n(1),n(0)) = e.w;
end

proc main
%  i1 = nodes.inv;

  W = map wMat to edges with nodes reduce +;
  nodes.outv = W * nodes.inv;

  outv = map eval to nodes;
  nodes.print = --outv;

%  nodes.outv = outv .* (1.0 - outv);
%  outv = map eval to nodes;
%  nodes.outv = outv;
%  d = map deriv to nodes;
%  dVdw = (d'*W)';
%  dVdw = W'*d;
%  nodes.d = dVdw;
end
  )";

  // Points
  Set<> nodes;
  FieldRef<double> outv = nodes.addField<double>("outv");
  FieldRef<double> inv = nodes.addField<double>("inv");
  FieldRef<double> d = nodes.addField<double>("d");
  FieldRef<double> print = nodes.addField<double>("print");

  // Springs
  Set<2> edges(nodes,nodes);
  FieldRef<double> w = edges.addField<double>("w");

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
      double rw = (ii*4.0+jj)/(4.0*l1)+0.01;
      w.set(edge, rw);
    }
  }

  std::cout << "Inv" << std::endl;
  for (auto &n : nodeRefs) {
    std::cout << (double)inv.get(n) << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Weights" << std::endl;
  for (auto &e : edgeRefs) {
    std::cout << (double)w.get(e) << std::endl;
  }
  std::cout << std::endl;

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> func = program.compile("main");
  if (!func) FAIL() << program.getDiagnostics().getMessage();

  func->bind("nodes", &nodes);
  func->bind("edges", &edges);
  func->runSafe();

  std::cout << "Check" << std::endl;
  for (auto &n : nodeRefs) {
    std::cout << (double)print.get(n) << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Outv" << std::endl;
  for (auto &n : nodeRefs) {
    std::cout << (double)outv.get(n) << std::endl;
  }

  // Check outputs
//  TensorRef<double,3> f0 = f.get(p0);
//  ASSERT_DOUBLE_EQ(2422.649730810374,  f0(0));
//  ASSERT_DOUBLE_EQ(2422.649730810374,  f0(1));
//  ASSERT_DOUBLE_EQ(2407.9347308103738, f0(2));
//
//  TensorRef<double,3> f1 = f.get(p1);
//  ASSERT_DOUBLE_EQ(-1961.3248654051868, f1(0));
//  ASSERT_DOUBLE_EQ(-1961.3248654051868, f1(1));
//  ASSERT_DOUBLE_EQ(-2029.9948654051868, f1(2));
//
//  TensorRef<double,3> f2 = f.get(p2);
//  ASSERT_DOUBLE_EQ(-461.3248654051871, f2(0));
//  ASSERT_DOUBLE_EQ(-461.3248654051871, f2(1));
//  ASSERT_DOUBLE_EQ(-480.9448654051871, f2(2));
}

#include "simit-test.h"


#include "graph.h"
#include "reorder.h"
#include "program.h"
#include "error.h"
#include "mesh.h"

#include <iostream>
#include <ostream>
#include <ctime>

using namespace std;
using namespace simit;
void vertexDataChecks(FieldRef<simit_float,3>& x, vector<ElementRef>& vertRefs, FieldRef<simit_float,3>& reorder_x, vector<ElementRef>& reorder_vertRefs,
    vector<int>& newOrdering) {

  for (int i = 0; i < newOrdering.size(); ++i) {
    SIMIT_ASSERT_FLOAT_NEAR_EQ(x.get(vertRefs[i])(0), reorder_x.get(reorder_vertRefs[newOrdering[i]])(0));
    SIMIT_ASSERT_FLOAT_NEAR_EQ(x.get(vertRefs[i])(1), reorder_x.get(reorder_vertRefs[newOrdering[i]])(1));
    SIMIT_ASSERT_FLOAT_NEAR_EQ(x.get(vertRefs[i])(2), reorder_x.get(reorder_vertRefs[newOrdering[i]])(2));
  }
}

void outputResults(ostream& os, FieldRef<simit_float,3>& x, vector<ElementRef>& vertRefs) {
  int counter = 0;
  for (auto& vert : vertRefs) {
    os << counter++ << " " << x.get(vert)(0) << " " << x.get(vert)(1) << " " << x.get(vert)(2) << endl;
  }
}

FieldRef<simit_float,3> initializeFem(MeshVol& mv, Set& m_verts, Set& m_tets, vector<ElementRef>& vertRefs) {
  FieldRef<simit_float,3>  x = m_verts.addField<simit_float,3>("x");
  FieldRef<simit_float,3>  v = m_verts.addField<simit_float,3>("v");
  // external forces
  FieldRef<simit_float,3> fe = m_verts.addField<simit_float,3>("fe");
  // constraints
  FieldRef<int>       c = m_verts.addField<int>("c");
  FieldRef<simit_float>    m = m_verts.addField<simit_float>("m");
  
  FieldRef<simit_float>    u = m_tets.addField<simit_float>("u");
  FieldRef<simit_float>    l = m_tets.addField<simit_float>("l");
  FieldRef<simit_float>    W = m_tets.addField<simit_float>("W");
  FieldRef<simit_float,3,3>B = m_tets.addField<simit_float,3,3>("B");
  
  simit_float uval, lval;
  // Youngs modulus and poisson's ratio
  simit_float E = 5e3;
  simit_float nu = 0.45;
  uval = 0.5*E/nu;
  lval = E*nu/((1+nu)*(1-2*nu));
  
  // create nodes, intial velocity and constraints
  simit_float initV[3]={0.1, 0.0, 0.1};
  simit_float eps = 0.0001;

  for(unsigned int ii =0 ;ii<mv.v.size(); ii++){
    vertRefs.push_back(m_verts.add());
    ElementRef p = vertRefs.back();
    x.set(p, {static_cast<simit_float>(mv.v[ii][0]),
              static_cast<simit_float>(mv.v[ii][1]),
              static_cast<simit_float>(mv.v[ii][2])});
    v.set(p, {initV[0], initV[1], initV[2]} );
    c.set(p,0);
    fe.set(p,{0.0,0.0,0.0});
    m.set(p, 0.0);
    if(mv.v[ii][1]<eps){
      c.set(p, 1);
      v.set(p, {0, 0, 0} );
    }
  }
  
  for(unsigned int ii =0 ;ii<mv.e.size(); ii++){
    ElementRef t = m_tets.add(
      vertRefs[mv.e[ii][0]],vertRefs[mv.e[ii][1]],
      vertRefs[mv.e[ii][2]],vertRefs[mv.e[ii][3]]
    );
    u.set(t,uval);
    l.set(t,lval);    
  }
  
  return x;
}

void loadAndRunFem(string& filename, Set& m_verts, Set& m_tets, const int nSteps) {
  Function m_precomputation;
  Function m_timeStepper;
  m_precomputation = loadFunction(filename, "initializeTet");

  m_precomputation.bind("verts", &m_verts);
  m_precomputation.bind("tets", &m_tets);
  m_precomputation.init();
  m_precomputation.unmapArgs();

  for (size_t i=0; i < nSteps; ++i) {
    m_precomputation.runSafe();
  }
  m_precomputation.mapArgs();  

  m_timeStepper = loadFunction(filename, "main");
  m_timeStepper.bind("verts", &m_verts);
  m_timeStepper.bind("tets", &m_tets);
  m_timeStepper.init();
  m_precomputation.unmapArgs();

  for (size_t i=0; i < nSteps; ++i) {
    m_timeStepper.runSafe();
  }
  m_timeStepper.mapArgs();
}

void femTest(string& filename, string& prefix, int nSteps) {
  string nodeFile = prefix + ".node";
  string eleFile = prefix + ".ele";
  MeshVol mv;
  mv.loadTet(nodeFile.c_str(), eleFile.c_str());
  Set m_verts;
  Set m_tets(m_verts,m_verts,m_verts,m_verts);
  vector<ElementRef> vertRefs;
  
  FieldRef<simit_float,3> x = initializeFem(mv, m_verts, m_tets, vertRefs); 
  
  clock_t begin = clock();
  loadAndRunFem(filename, m_verts, m_tets, nSteps); 
  clock_t end = clock(); 
  double randomTime = double(end - begin) / CLOCKS_PER_SEC;
  
  MeshVol reorder_mv;
  reorder_mv.loadTet(nodeFile.c_str(), eleFile.c_str());
  Set reorder_m_verts;
  Set reorder_m_tets(reorder_m_verts,reorder_m_verts,reorder_m_verts,reorder_m_verts);
  vector<ElementRef> reorder_vertRefs;
  
  FieldRef<simit_float,3> reorder_x = initializeFem(reorder_mv, reorder_m_verts, reorder_m_tets, reorder_vertRefs); 
  
  vector<int> vertexOrdering;
  vector<int> edgeOrdering;
  reorder_m_verts.setSpatialField("x");
  begin = clock();
  reorder(reorder_m_tets, reorder_m_verts, edgeOrdering, vertexOrdering);
  end = clock();
  double reorderTime = double(end - begin) / CLOCKS_PER_SEC;
  
  loadAndRunFem(filename, reorder_m_verts, reorder_m_tets, nSteps); 
  vertexDataChecks(x, vertRefs, reorder_x, reorder_vertRefs, vertexOrdering);
}
  
FieldRef<simit_float,3> initializeAverage(MeshVol& mv, Set& m_verts, Set& m_tets, vector<ElementRef>& vertRefs) {
  FieldRef<simit_float,3>  x = m_verts.addField<simit_float,3>("x");
  FieldRef<simit_float,3>  a = m_verts.addField<simit_float,3>("a");
  FieldRef<simit_float,3>    tx = m_tets.addField<simit_float,3>("x");
  
  for(unsigned int ii =0 ;ii<mv.v.size(); ii++){
    vertRefs.push_back(m_verts.add());
    ElementRef p = vertRefs.back();
    x.set(p, {static_cast<simit_float>(mv.v[ii][0]),
              static_cast<simit_float>(mv.v[ii][1]),
              static_cast<simit_float>(mv.v[ii][2])});
    a.set(p, {static_cast<simit_float>(mv.v[ii][0]),
              static_cast<simit_float>(mv.v[ii][1]),
              static_cast<simit_float>(mv.v[ii][2])});
  }
  
  for(unsigned int ii =0 ;ii<mv.e.size(); ii++){
    ElementRef t = m_tets.add(
      vertRefs[mv.e[ii][0]],vertRefs[mv.e[ii][1]],
      vertRefs[mv.e[ii][2]],vertRefs[mv.e[ii][3]]
    );
    tx.set(t, {static_cast<simit_float>(0.0),
              static_cast<simit_float>(0.0),
              static_cast<simit_float>(0.0)});
  }
  
  return x;
} 
  
TEST(Program, reorderInt) {
  Set m_verts;
  Set m_edges(m_verts,m_verts);
  vector<ElementRef> vertRefs;
  vector<ElementRef> edgeRefs;
  FieldRef<int,3>  x = m_verts.addField<int,3>("x");
  FieldRef<int,4>  a = m_verts.addField<int,4>("a");
  FieldRef<int,5>  tx = m_edges.addField<int,5>("x");
  
  Set reorder_m_verts;
  Set reorder_m_edges(reorder_m_verts, reorder_m_verts);
  vector<ElementRef> reorder_vertRefs;
  vector<ElementRef> reorder_edgeRefs;
  FieldRef<int,3>  reorder_x = reorder_m_verts.addField<int,3>("x");
  FieldRef<int,4>  reorder_a = reorder_m_verts.addField<int,4>("a");
  FieldRef<int,5>  reorder_tx = reorder_m_edges.addField<int,5>("x");
   
  for(unsigned int ii =0 ;ii<3; ii++) {
    vertRefs.push_back(m_verts.add());
    ElementRef p = vertRefs.back();
    x.set(p, {static_cast<int>(ii),
              static_cast<int>(ii),
              static_cast<int>(ii)});
    a.set(p, {static_cast<int>(3+ii),
              static_cast<int>(3+ii),
              static_cast<int>(3+ii),
              static_cast<int>(3+ii)});
    
    reorder_vertRefs.push_back(reorder_m_verts.add());
    p = reorder_vertRefs.back();
    reorder_x.set(p, {static_cast<int>(ii),
              static_cast<int>(ii),
              static_cast<int>(ii)});
    reorder_a.set(p, {static_cast<int>(3+ii),
              static_cast<int>(3+ii),
              static_cast<int>(3+ii),
              static_cast<int>(3+ii)});
  }
   
  ElementRef first = vertRefs[1];
  ElementRef second = vertRefs[2];
  edgeRefs.push_back(m_edges.add(first, second));
  ElementRef p = edgeRefs.back();
  tx.set(p, {static_cast<int>(20),
            static_cast<int>(20),
            static_cast<int>(20),
            static_cast<int>(20),
            static_cast<int>(20)});
  
  first = reorder_vertRefs[1];
  second = reorder_vertRefs[2];
  reorder_edgeRefs.push_back(reorder_m_edges.add(first, second));
  p = reorder_edgeRefs.back();
  reorder_tx.set(p, {static_cast<int>(20),
            static_cast<int>(20),
            static_cast<int>(20),
            static_cast<int>(20),
            static_cast<int>(20)});
  
  first = vertRefs[0];
  second = vertRefs[1];
  edgeRefs.push_back(m_edges.add(first, second));
  p = edgeRefs.back();
  tx.set(p, {static_cast<int>(10),
            static_cast<int>(10),
            static_cast<int>(10),
            static_cast<int>(10),
            static_cast<int>(10)});
  
  first = reorder_vertRefs[0];
  second = reorder_vertRefs[1];
  reorder_edgeRefs.push_back(reorder_m_edges.add(first, second));
  p = reorder_edgeRefs.back();
  reorder_tx.set(p, {static_cast<int>(10),
            static_cast<int>(10),
            static_cast<int>(10),
            static_cast<int>(10),
            static_cast<int>(10)});
  
  vector<int> vertexReordering {2, 0, 1};
  vector<int> edgeReordering {1,0};
  
  reorderVertexSet(reorder_m_edges, reorder_m_verts, vertexReordering);
  reorderEdgeSet(reorder_m_edges, edgeReordering);
  
  for (int i = 0; i < vertRefs.size(); ++i) {
    for (int j =0; j < 3; ++j) {
      auto incorrect = reorder_x.get(reorder_vertRefs[i])(j);
      auto actual = reorder_x.get(reorder_vertRefs[vertexReordering[i]])(j);
      auto expected = x.get(vertRefs[i])(j);
      ASSERT_NE(incorrect, expected);
      ASSERT_EQ(actual, expected); 
    }
  }
  
  for (int i = 0; i < vertRefs.size(); ++i) {
    for (int j =0; j < 4; ++j) {
      auto incorrect = reorder_a.get(reorder_vertRefs[i])(j);
      auto actual = reorder_a.get(reorder_vertRefs[vertexReordering[i]])(j);
      auto expected = a.get(vertRefs[i])(j);
      ASSERT_NE(incorrect, expected);
      ASSERT_EQ(actual, expected); 
    }
  }
   
  for (int i = 0; i < edgeRefs.size(); ++i) {
    for (int j = 0; j < 5; ++j) {
      auto incorrect = reorder_tx.get(reorder_edgeRefs[i])(j);
      auto actual = reorder_tx.get(reorder_edgeRefs[edgeReordering[i]])(j);
      auto expected = tx.get(edgeRefs[i])(j);
      ASSERT_NE(incorrect, expected);
      ASSERT_EQ(actual, expected); 
    }
  }
}

TEST(Program, reorderDouble) {
  Set m_verts;
  Set m_edges(m_verts,m_verts);
  vector<ElementRef> vertRefs;
  vector<ElementRef> edgeRefs;
  FieldRef<double,3>  x = m_verts.addField<double,3>("x");
  FieldRef<double,4>  a = m_verts.addField<double,4>("a");
  FieldRef<double,5>  tx = m_edges.addField<double,5>("x");
  
  Set reorder_m_verts;
  Set reorder_m_edges(reorder_m_verts, reorder_m_verts);
  vector<ElementRef> reorder_vertRefs;
  vector<ElementRef> reorder_edgeRefs;
  FieldRef<double,3>  reorder_x = reorder_m_verts.addField<double,3>("x");
  FieldRef<double,4>  reorder_a = reorder_m_verts.addField<double,4>("a");
  FieldRef<double,5>  reorder_tx = reorder_m_edges.addField<double,5>("x");
   
  for(unsigned int ii =0 ;ii<3; ii++) {
    vertRefs.push_back(m_verts.add());
    ElementRef p = vertRefs.back();
    x.set(p, {static_cast<double>(ii),
              static_cast<double>(ii),
              static_cast<double>(ii)});
    a.set(p, {static_cast<double>(3+ii),
              static_cast<double>(3+ii),
              static_cast<double>(3+ii),
              static_cast<double>(3+ii)});
    
    reorder_vertRefs.push_back(reorder_m_verts.add());
    p = reorder_vertRefs.back();
    reorder_x.set(p, {static_cast<double>(ii),
              static_cast<double>(ii),
              static_cast<double>(ii)});
    reorder_a.set(p, {static_cast<double>(3+ii),
              static_cast<double>(3+ii),
              static_cast<double>(3+ii),
              static_cast<double>(3+ii)});
  }
   
  ElementRef first = vertRefs[1];
  ElementRef second = vertRefs[2];
  edgeRefs.push_back(m_edges.add(first, second));
  ElementRef p = edgeRefs.back();
  tx.set(p, {static_cast<double>(20),
            static_cast<double>(20),
            static_cast<double>(20),
            static_cast<double>(20),
            static_cast<double>(20)});
  
  first = reorder_vertRefs[1];
  second = reorder_vertRefs[2];
  reorder_edgeRefs.push_back(reorder_m_edges.add(first, second));
  p = reorder_edgeRefs.back();
  reorder_tx.set(p, {static_cast<double>(20),
            static_cast<double>(20),
            static_cast<double>(20),
            static_cast<double>(20),
            static_cast<double>(20)});
  
  first = vertRefs[0];
  second = vertRefs[1];
  edgeRefs.push_back(m_edges.add(first, second));
  p = edgeRefs.back();
  tx.set(p, {static_cast<double>(10),
            static_cast<double>(10),
            static_cast<double>(10),
            static_cast<double>(10),
            static_cast<double>(10)});
  
  first = reorder_vertRefs[0];
  second = reorder_vertRefs[1];
  reorder_edgeRefs.push_back(reorder_m_edges.add(first, second));
  p = reorder_edgeRefs.back();
  reorder_tx.set(p, {static_cast<double>(10),
            static_cast<double>(10),
            static_cast<double>(10),
            static_cast<double>(10),
            static_cast<double>(10)});
  
  vector<int> vertexReordering {2, 0, 1};
  vector<int> edgeReordering {1,0};
  
  reorderVertexSet(reorder_m_edges, reorder_m_verts, vertexReordering);
  reorderEdgeSet(reorder_m_edges, edgeReordering);
  
  for (int i = 0; i < vertRefs.size(); ++i) {
    for (int j =0; j < 3; ++j) {
      auto incorrect = reorder_x.get(reorder_vertRefs[i])(j);
      auto actual = reorder_x.get(reorder_vertRefs[vertexReordering[i]])(j);
      auto expected = x.get(vertRefs[i])(j);
      ASSERT_NE(incorrect, expected);
      SIMIT_ASSERT_FLOAT_EQ(actual, expected); 
    }
  }
  
  for (int i = 0; i < vertRefs.size(); ++i) {
    for (int j =0; j < 4; ++j) {
      auto incorrect = reorder_a.get(reorder_vertRefs[i])(j);
      auto actual = reorder_a.get(reorder_vertRefs[vertexReordering[i]])(j);
      auto expected = a.get(vertRefs[i])(j);
      ASSERT_NE(incorrect, expected);
      SIMIT_ASSERT_FLOAT_EQ(actual, expected); 
    }
  }
   
  for (int i = 0; i < edgeRefs.size(); ++i) {
    for (int j = 0; j < 5; ++j) {
      auto incorrect = reorder_tx.get(reorder_edgeRefs[i])(j);
      auto actual = reorder_tx.get(reorder_edgeRefs[edgeReordering[i]])(j);
      auto expected = tx.get(edgeRefs[i])(j);
      ASSERT_NE(incorrect, expected);
      SIMIT_ASSERT_FLOAT_EQ(actual, expected); 
    }
  }
}

TEST(Program, reorderFemSpecificTest) {
  string dir(TEST_INPUT_DIR);
  string prefix=dir+"/program/fem/bar2k";
  string nodeFile = prefix + ".node";
  string eleFile = prefix + ".ele";
  size_t nSteps = 10;
  MeshVol mv;
  mv.loadTet(nodeFile.c_str(), eleFile.c_str());
  Set m_verts;
  Set m_tets(m_verts,m_verts,m_verts,m_verts);
  vector<ElementRef> vertRefs;
  
  FieldRef<simit_float,3> x = initializeFem(mv, m_verts, m_tets, vertRefs); 
  
  vector<int> vertexOrdering;
  vector<int> edgeOrdering;
  m_verts.setSpatialField("x");
  reorder(m_tets, m_verts, edgeOrdering, vertexOrdering);
    
  string filename = string(TEST_INPUT_DIR) + "/" +
                         toLower(test_info_->test_case_name()) + "/" +
                         "femTet.sim";
  loadAndRunFem(filename, m_verts, m_tets, nSteps); 

  int one = vertexOrdering[100];
  int two = vertexOrdering[200];
  int three = vertexOrdering[300];

  // Check outputs
  SIMIT_ASSERT_FLOAT_EQ(0.010771915616785779,  x.get(vertRefs[one])(0));
  SIMIT_ASSERT_FLOAT_EQ(0.058853573999788439,  x.get(vertRefs[one])(1));
  SIMIT_ASSERT_FLOAT_EQ(0.030899457015375883,  x.get(vertRefs[one])(2));
  SIMIT_ASSERT_FLOAT_EQ(0.0028221631202928516, x.get(vertRefs[two])(0));
  SIMIT_ASSERT_FLOAT_EQ(0.017969982607667911,  x.get(vertRefs[two])(1));
  SIMIT_ASSERT_FLOAT_EQ(0.012885386063393013,  x.get(vertRefs[two])(2));
  SIMIT_ASSERT_FLOAT_EQ(0.02411959295647129,   x.get(vertRefs[three])(0));
  SIMIT_ASSERT_FLOAT_EQ(0.052036155669135678,  x.get(vertRefs[three])(1));
  SIMIT_ASSERT_FLOAT_EQ(0.030173075240629205,  x.get(vertRefs[three])(2));
}

TEST(Program, reorderSquare) {
  string dir(TEST_INPUT_DIR);
  string prefix=dir+"/program/fem/square";
  string filename = string(TEST_INPUT_DIR) + "/" +
                         toLower(test_info_->test_case_name()) + "/" +
                         "femTet.sim";
  size_t nSteps = 10;
  femTest(filename, prefix, nSteps);
}

TEST(Program, reorderCube) {
  string dir(TEST_INPUT_DIR);
  string prefix=dir+"/program/fem/cube";
  string filename = string(TEST_INPUT_DIR) + "/" +
                         toLower(test_info_->test_case_name()) + "/" +
                         "femTet.sim";
  size_t nSteps = 10;
  femTest(filename, prefix, nSteps);
}

TEST(Program, reorderFemTest) {
  string dir(TEST_INPUT_DIR);
  string prefix=dir+"/program/fem/bar2k";
  string filename = string(TEST_INPUT_DIR) + "/" +
                         toLower(test_info_->test_case_name()) + "/" +
                         "femTet.sim";
  size_t nSteps = 10;
  femTest(filename, prefix, nSteps);
}

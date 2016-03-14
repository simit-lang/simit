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

void vertexDataChecks(FieldRef<simit_float,3>& x, vector<ElementRef>& vertRefs, 
    FieldRef<simit_float,3>& reorder_x, vector<ElementRef>& reorder_vertRefs,
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
  //external forces
  FieldRef<simit_float,3> fe = m_verts.addField<simit_float,3>("fe");
  //constraintss
  FieldRef<int>       c = m_verts.addField<int>("c");
  FieldRef<simit_float>    m = m_verts.addField<simit_float>("m");
  
  FieldRef<simit_float>    u = m_tets.addField<simit_float>("u");
  FieldRef<simit_float>    l = m_tets.addField<simit_float>("l");
  FieldRef<simit_float>    W = m_tets.addField<simit_float>("W");
  FieldRef<simit_float,3,3>B = m_tets.addField<simit_float,3,3>("B");
  
  simit_float uval, lval;
  //Youngs modulus and poisson's ratio
  simit_float E = 5e3;
  simit_float nu = 0.45;
  uval = 0.5*E/nu;
  lval = E*nu/((1+nu)*(1-2*nu));
  
  //create nodes, intial velocity and constraints
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
  
  cout << "Random Ordering took:    " << randomTime << " seconds" << endl;
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
  cout << "Reordering took:         " << reorderTime << " seconds" << endl;
  
  begin = clock();
  loadAndRunFem(filename, reorder_m_verts, reorder_m_tets, nSteps); 
  end = clock();
  double reorderedTime = double(end - begin) / CLOCKS_PER_SEC;
  cout << "Reordered Ordering took: " << reorderedTime << " seconds" << endl;
  cout << "Reordering Percentage:   " << (reorderTime / reorderedTime) * 100 << "%" << endl;
  cout << "Reordering Speedup:      " << ((randomTime - reorderedTime) / randomTime) * 100 << "%" << endl;
  
  vertexDataChecks(x, vertRefs, reorder_x, reorder_vertRefs, vertexOrdering);
}
  
FieldRef<simit_float,3> initializeAverage(MeshVol& mv, Set& m_verts, Set& m_tets, vector<ElementRef>& vertRefs) {
  simit::FieldRef<simit_float,3>  x = m_verts.addField<simit_float,3>("x");
  simit::FieldRef<simit_float,3>  a = m_verts.addField<simit_float,3>("a");
  
  simit::FieldRef<simit_float,3>    tx = m_tets.addField<simit_float,3>("x");
  
  for(unsigned int ii =0 ;ii<mv.v.size(); ii++){
    vertRefs.push_back(m_verts.add());
    simit::ElementRef p = vertRefs.back();
    x.set(p, {static_cast<simit_float>(mv.v[ii][0]),
              static_cast<simit_float>(mv.v[ii][1]),
              static_cast<simit_float>(mv.v[ii][2])});
    a.set(p, {static_cast<simit_float>(mv.v[ii][0]),
              static_cast<simit_float>(mv.v[ii][1]),
              static_cast<simit_float>(mv.v[ii][2])});
  }
  
  for(unsigned int ii =0 ;ii<mv.e.size(); ii++){
    simit::ElementRef t = m_tets.add(
      vertRefs[mv.e[ii][0]],vertRefs[mv.e[ii][1]],
      vertRefs[mv.e[ii][2]],vertRefs[mv.e[ii][3]]
    );
    tx.set(t, {static_cast<simit_float>(0.0),
              static_cast<simit_float>(0.0),
              static_cast<simit_float>(0.0)});
  }
  
  return x;
} 
  
void loadAndRunAverage(string& filename, Set& m_verts, Set& m_tets, const int nSteps) {
  Function m_timeStepper = loadFunction(filename, "main");
  m_timeStepper.bind("verts", &m_verts);
  m_timeStepper.bind("tets", &m_tets);
  m_timeStepper.init();
  
  for (size_t i=0; i < nSteps; ++i) {
    m_timeStepper.runSafe();
  }
  m_timeStepper.mapArgs();
}

void averageTest(string& filename, string& prefix, int nSteps) {
  string nodeFile = prefix + ".node";
  string eleFile = prefix + ".ele";
  MeshVol mv;
  mv.loadTet(nodeFile.c_str(), eleFile.c_str());
  Set m_verts;
  Set m_tets(m_verts,m_verts,m_verts,m_verts);
  vector<ElementRef> vertRefs;
  
  FieldRef<simit_float,3> edge_x = initializeAverage(mv, m_verts, m_tets, vertRefs); 
  
  clock_t begin = clock();
  loadAndRunAverage(filename, m_verts, m_tets, nSteps); 
  clock_t end = clock(); 
  double randomTime = double(end - begin) / CLOCKS_PER_SEC;
  
  cout << "Random Ordering took:    " << randomTime << " seconds" << endl;
  MeshVol reorder_mv;
  reorder_mv.loadTet(nodeFile.c_str(), eleFile.c_str());
  Set reorder_m_verts;
  Set reorder_m_tets(reorder_m_verts,reorder_m_verts,reorder_m_verts,reorder_m_verts);
  vector<ElementRef> reorder_vertRefs;
  
  FieldRef<simit_float,3> reorder_edge_x = initializeAverage(reorder_mv, reorder_m_verts, reorder_m_tets, reorder_vertRefs); 
  
  vector<int> vertexOrdering;
  vector<int> edgeOrdering;
  reorder_m_verts.setSpatialField("x");
  begin = clock();
  reorder(reorder_m_tets, reorder_m_verts, edgeOrdering, vertexOrdering);
  end = clock();
  double reorderTime = double(end - begin) / CLOCKS_PER_SEC;
  cout << "Reordering took:         " << reorderTime << " seconds" << endl;
  
  begin = clock();
  loadAndRunAverage(filename, reorder_m_verts, reorder_m_tets, nSteps); 
  end = clock();
  double reorderedTime = double(end - begin) / CLOCKS_PER_SEC;
  cout << "Reordered Ordering took: " << reorderedTime << " seconds" << endl;
  cout << "Reordering Percentage:   " << (reorderTime / reorderedTime) * 100 << "%" << endl;
  cout << "Reordering Speedup:      " << ((randomTime - reorderedTime) / randomTime) * 100 << "%" << endl;
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

TEST(Program, reorderDragon0) {
  string prefix="/data/scratch/ptew/random-graphs/dragon.0";
  string filename = string(TEST_INPUT_DIR) + "/" +
                         toLower(test_info_->test_case_name()) + "/" +
                         "femTet.sim";
  size_t nSteps = 1;
  femTest(filename, prefix, nSteps);
}

TEST(Program, reorderDragon1) {
  string prefix="/data/scratch/ptew/random-graphs/dragon.1";
  string filename = string(TEST_INPUT_DIR) + "/" +
                         toLower(test_info_->test_case_name()) + "/" +
                         "femTet.sim";
  size_t nSteps = 1;
  femTest(filename, prefix, nSteps);
}

TEST(Program, reorderAverage0) {
  string prefix="/data/scratch/ptew/random-graphs/dragon.0";
  string filename = string(TEST_INPUT_DIR) + "/" +
                         toLower(test_info_->test_case_name()) + "/" +
                         "averageTet.sim";
  size_t nSteps = 100;
  averageTest(filename, prefix, nSteps);
}

TEST(Program, reorderAverage1) {
  string prefix="/data/scratch/ptew/random-graphs/dragon.1";
  string filename = string(TEST_INPUT_DIR) + "/" +
                         toLower(test_info_->test_case_name()) + "/" +
                         "averageTet.sim";
  size_t nSteps = 100;
  averageTest(filename, prefix, nSteps);
}

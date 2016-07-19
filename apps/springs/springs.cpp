#include "graph.h"
#include "program.h"
#include "mesh.h"

#include <cmath>

using namespace simit;

int main(int argc, char **argv)
{
  if (argc != 3) {
    std::cerr << "Usage: springs <path to simit code> <path to data>" << std::endl;
    return -1;
  }
  std::string codefile = argv[1];
  std::string datafile = argv[2];

  simit::init("cpu", sizeof(double));

  // Load mesh data using Simit's mesh loader.
  MeshVol mesh;
  mesh.loadTet(datafile+".node", datafile+".ele");
  mesh.loadTetEdge(datafile+".edge");
  mesh.makeTetSurf();

  // Move the data to a floor and normalize to 1
  std::array< double,3> mn = mesh.v[0], mx = mesh.v[0];
  for(size_t vi = 1; vi < mesh.v.size(); vi++) {
    for(int i = 0; i < 3; i++) {
      mn[i] = std::min(mesh.v[vi][i], mn[i]);
      mx[i] = std::max(mesh.v[vi][i], mx[i]);
    }
  }
  for(size_t vi = 0; vi<mesh.v.size(); vi++) {
    for(int i = 0; i<3; i++) {
      mesh.v[vi][i] = (mesh.v[vi][i] - mn[i]) / (mx[i] - mn[i]);
    }
  }

  // Create a graph and initialize it with mesh data
  Set points;
  Set springs(points, points);

  double stiffness = 1e4;
  double density   = 1e3;
  double radius    = 0.01;
  double pi        = 3.14159265358979;
  double zfloor    = 0.1;               // we fix everything below the floor

  // The fields of the points set 
  FieldRef<double,3> x     = points.addField<double,3>("x");
  FieldRef<double,3> v     = points.addField<double,3>("v");
  FieldRef<double>   m     = points.addField<double>("m");
  FieldRef<bool>     fixed = points.addField<bool>("fixed");

  // The fields of the springs set 
  FieldRef<double> k  = springs.addField<double>("k");
  FieldRef<double> l0 = springs.addField<double>("l0");

  std::vector<ElementRef> pointRefs;
  for(auto vertex : mesh.v) {
    ElementRef point = points.add();
    pointRefs.push_back(point);

    x.set(point, vertex);
    v.set(point, {0.0, 0.0, 0.0});
    fixed.set(point, vertex[2] < zfloor);
  }

  // Compute point masses
  std::vector<double> pointMasses(mesh.v.size(), 0.0);

  for(auto e : mesh.edges) {
    double dx[3];
    double *x0 = &(mesh.v[e[0]][0]);
    double *x1 = &(mesh.v[e[1]][0]);
    dx[0] = x1[0] - x0[0];
    dx[1] = x1[1] - x0[1];
    dx[2] = x1[2] - x0[2];
    double l0_ = sqrt(dx[0]*dx[0] + dx[1]*dx[1] + dx[2]*dx[2]);
    double vol = pi*radius*radius*l0_;
    double mass = vol*density;
    pointMasses[e[0]] += 0.5*mass;
    pointMasses[e[1]] += 0.5*mass;
    ElementRef spring = springs.add(pointRefs[e[0]], pointRefs[e[1]]);
    l0.set(spring, l0_);
    k.set(spring, stiffness);
  }
  for(size_t i = 0; i < mesh.v.size(); ++i) {
    ElementRef p = pointRefs[i];
    m.set(p, pointMasses[i]);
  }

  // Compile program and bind arguments
  Program program;
  program.loadFile(codefile);

  Function timestep = program.compile("timestep");
  timestep.bind("points",  &points);
  timestep.bind("springs", &springs);

  timestep.init();

  // Take 100 time steps
  for (int i = 1; i <= 100; ++i) {
    std::cout << "timestep " << i << std::endl;

    timestep.unmapArgs(); // Move data to compute memory space (e.g. GPU)
    timestep.run();       // Run the timestep function
    timestep.mapArgs();   // Move data back to this memory space

    // Copy the x field to the mesh and save it to an obj file
    int vi = 0;
    for (auto &vert : points) {
      for(int ii = 0; ii < 3; ii++){
        mesh.v[vi][ii] = x.get(vert)(ii);
      }
      vi++;
    }
    mesh.updateSurfVert();
    mesh.saveTetObj(std::to_string(i)+".obj");
  }
}

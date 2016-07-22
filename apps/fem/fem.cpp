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
  mesh.makeTetSurf();


  // Create a graph and initialize it with mesh data
  Set verts;
  Set tets(verts, verts, verts, verts);

  simit::FieldRef<double,3>   x  = verts.addField<double,3>("x");
  simit::FieldRef<double,3>   v  = verts.addField<double,3>("v");
  simit::FieldRef<double,3>   fe = verts.addField<double,3>("fe");

  simit::FieldRef<int>        c  = verts.addField<int>("c");
  simit::FieldRef<double>     m  = verts.addField<double>("m");
  
  simit::FieldRef<double>     u  = tets.addField<double>("u");
  simit::FieldRef<double>     l  = tets.addField<double>("l");
  simit::FieldRef<double>     W  = tets.addField<double>("W");
  simit::FieldRef<double,3,3> B  = tets.addField<double,3,3>("B");


  // Compile program and bind arguments
  Program program;
  program.loadFile(codefile);

  Function precompute = program.compile("initializeTet");

  Function timestep   = program.compile("main");
  timestep.bind("verts", &verts);
  timestep.bind("tets",  &tets);

  timestep.init();


  // Create a graph and initialize it with mesh data
  Set points;
  Set springs(points, points);

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

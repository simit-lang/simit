#ifndef SIMIT_MESH_H
#define SIMIT_MESH_H

#include <iostream>
#include <vector>
#include <array>
namespace simit{

///a triagular mesh data structure for loading
///plain text obj files. Does not work with quad mesh.
///Assumes one object per file.
///Only reads vertex and face and ignores all other attributes.
struct Mesh{
  ///vertex list
  std::vector<std::array< double,3> > v;
  ///triangle list
  std::vector<std::array< int,3> > t;

  ///return -1 if failed to load
  int load(const char * filename);
  ///return -1 if failed to load or format is unrecognized
  int load(std::istream & in);
  ///return -1 if failed to save
  int save(const char * filename);
  int save(std::ostream & out);
};

///a struct used to load custom volumetric mesh file
struct MeshVol{

  
};

}

#endif
 

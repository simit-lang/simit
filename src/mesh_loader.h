#ifndef SIMIT_MESH_LOADER_H
#define SIMIT_MESH_LOADER_H

#include <string>
#include "graph.h"

namespace simit {

int loadMesh(const std::string &path, simit::Set<> *points,
              simit::Set<3> *faces);

int loadMesh(std::istream &input, simit::Set<> *points,
              simit::Set<3> *faces);

int storeMesh(const std::string &path, simit::Set<> &points,
               simit::Set<3> &faces);

int storeMesh(std::ostream &output, simit::Set<> &points,
               simit::Set<3> &faces);

}

#endif // SIMIT_MESH_LOADER_H

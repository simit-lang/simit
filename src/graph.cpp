#include "graph.h"

#include <iostream>
#include "graph_indices.h"

using namespace std;

namespace simit {

Set::~Set() {
  for (auto f: fields) {
    delete f;
  }
  free(endpoints);
  free(latticePoints);
  free(latticeLinks);

  delete this->neighbors;
}

void Set::increaseCapacity() {
  for (auto f : fields) {
    int typeSize = f->sizeOfType;
    f->data = realloc(f->data, (capacity+capacityIncrement) * typeSize);
    memset((char*)(f->data)+capacity*typeSize, 0, capacityIncrement*typeSize);

    for (FieldRefBase *fieldRef : f->fieldReferences) {
      fieldRef->data = f->data;
    }
  }
  capacity += capacityIncrement;
}

const internal::NeighborIndex *Set::getNeighborIndex() const {
  iassert(getCardinality() > 0) << "Vertex sets have no neighbor index.";

  tassert(isHomogeneous())
      << "neighbor indices are currently only supported for homogeneous sets";

  // Lazy allocation
  if (this->neighbors == nullptr) {
    this->neighbors = new internal::NeighborIndex(*this);
  }

  return this->neighbors;
}


// Graph generators
void createElements(Set *elements, unsigned num) {
  for (size_t i=0; i < num; ++i) {
    elements->add();
  }
}

#define node0(x,y,z)  x*numY*numZ + y*numZ + z      // node at x,y,z
#define node1X(x,y,z) (x+1)*numY*numZ + y*numZ + z  // x's neighbor
#define node1Y(x,y,z) x*numY*numZ + (y+1)*numZ + z  // y's neighbor
#define node1Z(x,y,z) x*numY*numZ + y*numZ + (z+1)  // z's neighbor

Box::Box(unsigned nX, unsigned nY, unsigned nZ,
    std::vector<ElementRef> refs, std::map<Box::Coord, ElementRef> coords2edges)
    : nX(nX), nY(nY), nZ(nZ), refs(refs), coords2edges(coords2edges) {
  iassert(refs.size() == nX*nY*nZ);
}

ElementRef Box::getEdge(ElementRef p1, ElementRef p2) const {
  Coord coord(p1,p2);
  if (coords2edges.find(coord) == coords2edges.end()) {
    return ElementRef();
  }
  return coords2edges.at(coord);
}

std::vector<ElementRef> Box::getEdges() {
  std::vector<ElementRef> edges;
  for (auto &coord2edge : coords2edges) {
    edges.push_back(coord2edge.second);
  }
  return edges;
}

Box createBox(Set *vertices, Set *edges,
              unsigned numX, unsigned numY, unsigned numZ) {
  uassert(numX >= 1 && numY >= 1 && numZ >= 1);
  vector<ElementRef> points(numX*numY*numZ);

  for(unsigned x = 0; x < numX; ++x) {
    for(unsigned y = 0; y < numY; ++y) {
      for(unsigned z = 0; z < numZ; ++z) {
        points[node0(x,y,z)] = vertices->add();
      }
    }
  }

  map<Box::Coord, ElementRef> coords2edges;

  // x edges
  for(unsigned x = 0; x < numX-1; ++x) {
    for(unsigned y = 0; y < numY; ++y) {
      for(unsigned z = 0; z < numZ; ++z) {
        Box::Coord coord(points[node0(x,y,z)], points[node1X(x,y,z)]);
        simit::ElementRef edge = edges->add(coord.first, coord.second);
        coords2edges[coord] = edge;
      }
    }
  }

  // y edges
  for(unsigned x = 0; x < numX; ++x) {
    for(unsigned y = 0; y < numY - 1; ++y) {
      for(unsigned z = 0; z < numZ; ++z) {
        Box::Coord coord(points[node0(x,y,z)], points[node1Y(x,y,z)]);
        simit::ElementRef edge = edges->add(coord.first, coord.second);
        coords2edges[coord] = edge;
      }
    }
  }

  // z edges
  for(unsigned x = 0; x < numX; ++x) {
    for(unsigned y = 0; y < numY; ++y) {
      for(unsigned z = 0; z < numZ-1; ++z) {
        Box::Coord coord(points[node0(x,y,z)], points[node1Z(x,y,z)]);
        simit::ElementRef edge = edges->add(coord.first, coord.second);
        coords2edges[coord] = edge;
      }
    }
  }

  return Box(numX, numY, numZ, points, coords2edges);
}

} // namespace simit

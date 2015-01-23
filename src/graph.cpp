#include "graph.h"

#include <iostream>
#include "indices.h"

using namespace std;

namespace simit {

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
  tassert(isHomogeneous())
      << "neighbor indices are currently only supported for homogeneous sets";

  if (getCardinality() >= 2 && neighbors == nullptr) {
    // Cast to non-const since adding a neighbor index does not change the 
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

Box createBox(Set *elements, Set *edges,
              unsigned numX, unsigned numY, unsigned numZ) {
  vector<ElementRef> points(numX*numY*numZ);

  for(unsigned x = 0; x < numX; ++x) {
    for(unsigned y = 0; y < numY; ++y) {
      for(unsigned z = 0; z < numZ; ++z) {
        points[node0(x,y,z)] = elements->add();
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

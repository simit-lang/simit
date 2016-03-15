#include "reorder.h"
#include "graph.h"
#include "hilbert.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <cfloat>
#include <string>

using namespace std;
namespace simit {

  // ---------- Hilbert Reordering Heuristic ----------
  namespace hilbert {
    // This function populates the hilbertId field of each vertex_t in nodes, by
    // remapping every vertex onto an n^3 lattice using appropriate scaling factors,
    // and then traversing the lattice using a 3-D Hilbert curve.
    //
    // hilbertBits = number of bits in Hilbert grid (grid with side 2^hilbertBits)
    void assignHilbertIds(vertex_t * const nodes, const int cntNodes,
                          const unsigned hilbertBits) {
      // We first traverse all vertices to find the maximal and minimal coordinates
      // along each axis: xMin, xMax, yMin, yMax, zMin, zMax.

      double xMin, xMax, yMin, yMax, zMin, zMax;
      uint64_t hilbertGridN = 1 << hilbertBits; 
      xMin = yMin = zMin = DBL_MAX;
      xMax = yMax = zMax = DBL_MIN;
      for (int i = 0; i < cntNodes; ++i) {
        xMin = fmin(xMin, nodes[i].x);
        yMin = fmin(yMin, nodes[i].y);
        zMin = fmin(zMin, nodes[i].z);
        xMax = fmax(xMax, nodes[i].x);
        yMax = fmax(yMax, nodes[i].y);
        zMax = fmax(zMax, nodes[i].z);
      }

      // We now create a mapping that maps:
      //   (xMin, yMin, zMin) to lattice point (0, 0, 0)
      //   (xMax, yMax, zMax) to lattice point (n-1, n-1, n-1)
      // where n = hilbertGridN
      //
      // This mapping is the following: for the T axis,
      // tLattice = round((t - tMin) * (hilbertGridN - 1) / tMax);

      for (int i = 0; i < cntNodes; ++i) {
        bitmask_t latticeCoords[3];
        bitmask_t hilbertIndex;

        nodes[i].id = (vid_t)i;

        latticeCoords[0] = (uint64_t) round((nodes[i].x - xMin) * (hilbertGridN - 1) / xMax);
        latticeCoords[1] = (uint64_t) round((nodes[i].y - yMin) * (hilbertGridN - 1) / yMax);
        latticeCoords[2] = (uint64_t) round((nodes[i].z - zMin) * (hilbertGridN - 1) / zMax);

        hilbertIndex = hilbert_c2i(3, hilbertBits, latticeCoords);
        nodes[i].hilbertId = (vid_t) hilbertIndex;

      }
    }

    bool vertexComparator(const vertex_t& a, const vertex_t& b) {
      if (a.hilbertId != b.hilbertId) {
        return a.hilbertId < b.hilbertId;
      } else {
        return a.id < b.id;
      }
    }

    static void createIdTranslationMapping(vertex_t * reorderedNodes, vector<int>& vertexOrdering, int cntNodes) {
      assert(vertexOrdering.size() != 0);
      vertexOrdering.resize(cntNodes);

      for (int i = 0; i < cntNodes; ++i) {
        vertexOrdering[reorderedNodes[i].id] = i;
      }
    }

    void loadNodes(Set& vertexSet, vertex_t ** outNodes, int numNodes) {
      vertex_t * nodes = new (nothrow) vertex_t[numNodes];
      
      auto& fields = vertexSet.getFields();
      int fieldIndex = vertexSet.getFieldIndex(vertexSet.getSpatialFieldName());
      double * spatialData = static_cast<double*>(fields[fieldIndex]->data);  
     
      for (int i = 0; i < numNodes; ++i) {
        nodes[i].id = i; 
        nodes[i].x = spatialData[i*3+0];
        nodes[i].y = spatialData[i*3+1];
        nodes[i].z = spatialData[i*3+2];
      }
      
      *outNodes = nodes;
    }

    void hilbertReorder(Set& vertexSet, vector<int>& vertexOrdering) {
      vertex_t * nodes;
      const int hilbertBits = 8;  
      int cntNodes = vertexSet.getSize();
      loadNodes(vertexSet, &nodes, cntNodes);

      assignHilbertIds(nodes, cntNodes, hilbertBits);
      stable_sort(nodes, nodes + cntNodes, vertexComparator);
      createIdTranslationMapping(nodes, vertexOrdering, cntNodes);
    }
  } // namespace simit::hilbert
 
  // ---------- Simit Level Reordering Heuristics ----------
  int qsortCompare( const void* a, const void* b) {
       int int_a = * ( (int*) a );
       int int_b = * ( (int*) b );

       if ( int_a == int_b ) return 0;
       else if ( int_a < int_b ) return -1;
       else return 1;
  } 
  
  struct edgeCompare{
    edgeCompare(int* endpoints, const int cardinality) : 
      endpoints(endpoints),
      cardinality(cardinality)
      {}
    
    bool operator()(int const&left, 
                    int const&right) const {
      
      for (int i=0; i < cardinality; ++i) {
        int leftID = endpoints[left*cardinality + i];
        int rightID = endpoints[right*cardinality + i];
        if (leftID != rightID) {
          return leftID < rightID; 
        }
      }
      return true;
    }
    private:
      int* endpoints;
      const int cardinality;
  };

  void edgeVertexSortReordering(Set& edgeSet, vector<int>& edgeOrdering) {
    int* endpoints = edgeSet.getEndpointsPtr();
    const int size = edgeSet.getSize();
    const int cardinality = edgeSet.getCardinality();
    
    assert(edgeOrdering.size() == 0);
    edgeOrdering.resize(size);
    int* sortableEndpoints = static_cast<int *>(malloc(size * cardinality * sizeof(int)));
    memcpy(sortableEndpoints, endpoints, size * cardinality * sizeof(int));

    for (int index=0; index < size; ++index) {
      edgeOrdering[index] = index;
      qsort(sortableEndpoints+ index*cardinality, cardinality, sizeof(int), qsortCompare);
    } 
    
    sort(edgeOrdering.begin(), edgeOrdering.end(), edgeCompare(sortableEndpoints, cardinality));
    free(sortableEndpoints);
  }

  // ---------- Reordering Helper Functions ----------
  void reorderFields(vector<Set::FieldData*>& fields, const vector<int>& ordering) {
    for (auto f : fields) {
      switch (f->type->getComponentType()) {
        case ComponentType::Float: {
          float* data = static_cast<float *>(f->data);
          reorderFieldData(data, ordering, f->sizeOfType);
          break;
        }
        case ComponentType::Double: {
          double* data = static_cast<double *>(f->data);
          reorderFieldData(data, ordering, f->sizeOfType);
          break;
        }
        case ComponentType::Int: {
          int* data = static_cast<int *>(f->data);
          reorderFieldData(data, ordering, f->sizeOfType);
          break;
        }
        case ComponentType::Boolean: {
          bool* data = static_cast<bool *>(f->data);
          reorderFieldData(data, ordering, f->sizeOfType);
        }
      }
    }
  }
  
  void reorderEdgeSet(Set& edgeSet, const vector<int>& edgeOrdering) {
    int* endpoints = edgeSet.getEndpointsPtr();
    const int size = edgeSet.getSize();
    const int cardinality = edgeSet.getCardinality();

    int* newEndpoints = static_cast<int *>(malloc(size * cardinality * sizeof(int)));
    memcpy(newEndpoints, endpoints, size * cardinality * sizeof(int));

    for (int edgeIndex=0; edgeIndex < size; ++edgeIndex) {
      iassert(edgeIndex < edgeOrdering.size());
      iassert(edgeOrdering[edgeIndex] < (size - 1) * cardinality * sizeof(int));
      memcpy(newEndpoints + edgeIndex * cardinality, endpoints + edgeOrdering[edgeIndex] * cardinality, cardinality * sizeof(int));
    }
    memcpy(endpoints, newEndpoints, size * cardinality * sizeof(int));
    free(newEndpoints);
    
    iassert(edgeOrdering.size() == edgeSet.getSize()) << edgeOrdering.size() << ", " << edgeSet.getSize();
    reorderFields(edgeSet.getFields(), edgeOrdering);
  }

  void reorderVertexSet(Set& edgeSet, Set& vertexSet, vector<int>& vertexOrdering) {
    // Reset Endpoints to reflect reordering
    // Vertex ordering maps old to new identity 
    // This itertates over all enpoints translating from old to new
    for (int i=0; i < edgeSet.getSize() * edgeSet.getCardinality(); ++i) {
      iassert(vertexOrdering[edgeSet.getEndpointsPtr()[i]] < vertexSet.getSize());
      edgeSet.getEndpointsPtr()[i] = vertexOrdering[edgeSet.getEndpointsPtr()[i]]; 
    }
    
    iassert(vertexOrdering.size() == vertexSet.getSize()) << vertexOrdering.size() << ", " << vertexSet.getSize();
    reorderFields(vertexSet.getFields(), vertexOrdering);
  }
  
  void reorder(Set& edgeSet, Set& vertexSet, vector<int>& edgeOrdering, vector<int>& vertexOrdering) {
    iassert(vertexOrdering.size() == 0) << "Vertex Ordering needs to be initially empty";
    iassert(edgeOrdering.size() == 0) << "Edge Ordering needs to be initially empty";
    iassert(vertexSet.hasSpatialField()) << "Vertex Set must have a spatial field set prior to reordering";
    
    // Get new vertex ordering based on given heuristic 
    hilbert::hilbertReorder(vertexSet, vertexOrdering);
    reorderVertexSet(edgeSet, vertexSet, vertexOrdering);

    // Get new edge ordering based on given heuristic 
    edgeVertexSortReordering(edgeSet, edgeOrdering); 
    reorderEdgeSet(edgeSet, edgeOrdering);
  }
  
  void reorder(Set& edgeSet, Set& vertexSet) {
    vector<int> vertexOrdering;
    vector<int> edgeOrdering;
    reorder(edgeSet, vertexSet, edgeOrdering, vertexOrdering);
  }
}

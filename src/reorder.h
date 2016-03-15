#ifndef SIMIT_REORDER_H
#define SIMIT_REORDER_H

#include <graph.h>
#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <fstream>

namespace simit {
  
  void reorderVertexSet(Set& edgeSet, Set& vertexSet, std::vector<int>& vertexOrdering);
  void reorderEdgeSet(Set& edgeSet, const std::vector<int>& edgeOrdering);
  void reorderEdgeSetByVertexOrdering(Set& edgeSet, const std::vector<int>& vertexOrdering);

  void reorder(Set& edgeSet, Set& vertexSet);
  void reorder(Set& edgeSet, Set& vertexSet, std::vector<int>& edgeOrdering, std::vector<int>& vertexOrdering);
 
  template<typename T>
  void reorderFieldData(T* data, const std::vector<int>& vertexOrdering, const int typeSize) {
    const int capacity = vertexOrdering.size(); 
    T* newData = static_cast<T*>(malloc(capacity * typeSize));
    int dim = typeSize/sizeof(T);
    assert(dim > 0);
    for (int i=0; i < capacity; ++i) {
      for (int x=0; x < dim; ++x) {
        assert(vertexOrdering[i]*dim + x < capacity * dim);
        newData[vertexOrdering[i]*dim+x] = data[i*dim+x];
      }
    }

    memcpy(data, newData, capacity * typeSize); 
    free(newData);
  }

  namespace hilbert {
    
    typedef uint64_t vid_t;  // vertex id type

    struct edges_t {
      vid_t cntEdges;
      vid_t * edges;
    };
    typedef struct edges_t edges_t;

    struct vertex_t {
      vid_t id;
      vid_t hilbertId;
      double x;
      double y;
      double z;
      void * data;
      edges_t edgeData;
    };
    typedef struct vertex_t vertex_t;

    typedef std::pair<vid_t, vid_t> edge_t;

    void hilbertReorder(Set& vertexSet, std::vector<int>& vertexOrdering, int vertexCount);
  } // namespace simit::hilbert

} // namespace simit 
#endif

#ifndef SIMIT_INDICES_H
#define SIMIT_INDICES_H

#include "graph.h"
#include <map>
#include <vector>

namespace simit {
namespace internal {

/// A class for an index that maps from points to edges that contain that point
/// differentiating between different endpoints
class VertexToEdgeEndpointIndex {
 public:
  VertexToEdgeEndpointIndex(const SetBase &edgeSet);
 ~VertexToEdgeEndpointIndex();
  
  int getNumEdgesForElement(ElementRef vertex, int whichEndpoint) {
    return numEdgesForVertex[whichEndpoint][vertex.ident];
  }
  
  int* getWhichEdgesForElement(ElementRef vertex, int whichEndpoint) {
    return whichEdgesForVertex[whichEndpoint][vertex.ident];
  }
  
  std::vector<int*> getNumEdges() { return numEdgesForVertex; }
  
  std::vector<int**> getWhichEdges() { return whichEdgesForVertex; }
  
  int getTotalEdges() { return totalEdges; }

 private:
  std::vector<const SetBase*> endpointSets;       // the endpoint sets
  std::vector<int*> numEdgesForVertex;      // number of edges the v belongs to
  std::vector<int**> whichEdgesForVertex;   // which edges v belongs to
  int totalEdges;
};


/// A class for an index that maps from points to edges that contain that point
/// with no differentiation by endpoint
class VertexToEdgeIndex {
 public:
  VertexToEdgeIndex(const SetBase &edgeSet);
  ~VertexToEdgeIndex();
  
  int getNumEdgesForElement(ElementRef vertex, const SetBase& whichSet) {
    return numEdgesForVertex[&whichSet][vertex.ident];
  }
  
  int* getWhichEdgesForElement(ElementRef vertex, const SetBase& whichSet) {
    return whichEdgesForVertex[&whichSet][vertex.ident];
  }
  
  std::map<const SetBase*,int*> getNumEdges() { return numEdgesForVertex; }
  
  std::map<const SetBase*,int**> getWhichEdges() { return whichEdgesForVertex; }
  
  int getTotalEdges() { return totalEdges; }
  
 private:
  std::vector<const SetBase*> endpointSets;           // the endpoint sets
  std::map<const SetBase*,int*> numEdgesForVertex;    // number of edges the v
                                                      // belongs to
  std::map<const SetBase*,int**> whichEdgesForVertex; // which edges v belongs to
  int totalEdges;
};


/// Maps elements to their neighbors through an edge set. Note that an element
/// is its own neighbor. This index does not work for heterogeneous graphs.
class NeighborIndex {
 public:
  NeighborIndex(const SetBase &edgeSet);
  ~NeighborIndex();
  
  int getNumNeighbors(ElementRef vertex) const {
    return startIndex[vertex.ident+1] - startIndex[vertex.ident];
  }

  int getSize() const {
    return neighbors.size();
  }

  // Get a pointer to the neighbors of the given element.
  const int* getNeighbors(ElementRef element) const {
    return &neighbors[startIndex[element.ident]];
  }

  const int* getStartIndex() const { return startIndex; }
  
  const int* getNeighborIndex() const { return neighbors.data(); }
  
 private:
  /// start index into neighbors array for vertex.
  /// the last index is total size of neighbors array, which is also the number
  /// of non-zeros in a vertex x vertex matrix.
  int* startIndex;

  /// which edges v belongs to
  std::vector<int> neighbors;

  void addNoCollision(int x, std::vector<int> & a);
};

}} // simit::internal
#endif

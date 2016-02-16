#include "reorder.h"

#include "graph.h"
#include "hilbert.h"
#include <omp.h>
#include <algorithm>
#include <map>
#include <vector>
#include <list>
#include <unordered_map>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <climits>
#include <cfloat>
#include <string>
#include <algorithm>
#include <deque>

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

      #pragma omp parallel for
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

      #pragma omp parallel for
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
  struct ascendingCompare {
      bool operator()(pair< int, int> const&left, 
                      pair< int, int> const&right) const {
          return left.second < right.second;
      }
  };
  
  struct descendingCompare {
      bool operator()(pair< int, int> const&left, 
                      pair< int, int> const&right) const {
          return left.second > right.second;
      }
  };
  
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
  
  void vertexDegreeReordering(vector<int>& vertexOrdering, int* endpoints, int size, int cardinality) {
    unordered_map<int,int> degrees;
    int edgeIndex;

    for (int i=0; i < size; ++i) {
      edgeIndex = i * cardinality;
      for (int elementIndex=0; elementIndex < cardinality; ++elementIndex) {
        auto search = degrees.find(endpoints[edgeIndex*cardinality + elementIndex]);
        if ( search == degrees.end() ) {
          degrees[endpoints[edgeIndex*cardinality + elementIndex]] = 0;
        } 
        degrees[endpoints[edgeIndex*cardinality + elementIndex]] += 1;
      }
    }
    vector<pair<int, int> > degreeVec(degrees.begin(), degrees.end());
    sort(degreeVec.begin(), degreeVec.end(), descendingCompare());
    
    for (auto& p : degreeVec) {
      vertexOrdering.push_back(p.first);
    }
  }

  void edgeSumReordering(int* endpoints, vector<int>& edgeOrdering, const int size, const int cardinality) {
    assert(edgeOrdering.size() == 0);
    unordered_map<int,int> edgeSum;
    
    for (int i=0; i < size; ++i) {
      int edgeIndex = i * cardinality;
      int sum = 0;
      for (int elementIndex=0; elementIndex < cardinality; ++elementIndex) {
        sum += endpoints[edgeIndex + elementIndex];
      }
      edgeSum[i] = sum;
    }
    
    vector<pair<int, int> > sumVec(edgeSum.begin(), edgeSum.end());
    sort(sumVec.begin(), sumVec.end(), ascendingCompare());

    for (auto& p : sumVec) {
      edgeOrdering.push_back(p.first);
    }
  }

  void edgeVertexSortReordering(int* endpoints, vector<int>& edgeOrdering, const int size, const int cardinality) {
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
  
  // ---------- Strip Reordering ----------

  struct BestUnvisited {
    BestUnvisited(vector<bool>& visited) : 
      visited(visited)
      {}
    
    bool operator()(pair<int, vector<int>> const&left, 
                    pair<int, vector<int>> const&right) const {
      if (visited[left.first]) {
        return false;
      }
      return numberOfUnvisited(left.second) < numberOfUnvisited(right.second);  
    }
    
    private:
      const vector<bool>& visited;
      int numberOfUnvisited(const vector<int>& edges) const {
        int sum = 0;
        for (auto& edge : edges) {
          if (!visited[edge]) {
            sum++;
          }
        }
        return sum;
      }
  };
  
  struct CounterClockwise {
    const double* spatialData;
    const double x;
    const double y;
    const double z;
    
    CounterClockwise(double* spatialData, int currentEdge) : 
      spatialData(spatialData),
      x(spatialData[currentEdge*3]),
      y(spatialData[currentEdge*3+1]),
      z(spatialData[currentEdge*3+2])
    {}
    
    bool operator()(const int left, 
                    const int right) const {
      return angle(left) > angle(right); 
    }
    
    double angle(const int edge) const {
      double edgeX = spatialData[edge*3];
      double edgeY = spatialData[edge*3+1];
      double edgeZ = spatialData[edge*3+2]; 
      if (edgeX == x) {
        return 0.0;
      } else {
        return 90.0 - (atan2(edgeY - y, edgeX - x) * 180.0 / M_PI);
      }
    }
  };
  
  bool addNeighborsByCC(double* spatialData, vector<int>& neighbors, deque<int>& edgeQueue, vector<bool> visited, const int currentEdge) {
    sort(neighbors.begin(), neighbors.end(), CounterClockwise(spatialData, currentEdge));
    bool added = false;
    for (auto& neighbor : neighbors) {
      if (!visited[neighbor]) {
        edgeQueue.push_front(neighbor);
        added = true;
      }
    }
    return added;
  }

  void greedyStripReordering(Set& edgeSet, vector<int>& edgeReordering, map<int, vector<int>> edgeNeighbors) {
    assert(edgeReordering.size() == 0);
    int STRIP_SIZE = 64;
    int currentEdge = -1;
    vector<int> currentNeighbors;
    vector<bool> visited(edgeSet.getSize(), false);
    
    auto& edgeFields = edgeSet.getFields();
    int spatialFieldIndex = edgeSet.getFieldIndex(edgeSet.getSpatialFieldName());
    double * edgeSpatialData = static_cast<double*>(edgeFields[spatialFieldIndex]->data); 
    while (edgeReordering.size() < edgeSet.getSize()) {
      while (currentEdge < 0 || visited[currentEdge]) {
        currentEdge = min_element(edgeNeighbors.begin(), edgeNeighbors.end(), BestUnvisited(visited))->first;
        currentNeighbors = edgeNeighbors[currentEdge];
        edgeNeighbors.erase(currentEdge);
      }
        
      assert(!visited[currentEdge]);
      int stripLength = 0;
      deque<int> edgeQueue;
      bool buildStrip = true;
      while (buildStrip && !visited[currentEdge]) {
        buildStrip = stripLength != STRIP_SIZE;

        if (buildStrip) {
          visited[currentEdge] = true;
          edgeReordering.push_back(currentEdge);
          stripLength++;
          
          // This might be wrong. Is correct if the queue is empty before this.
          buildStrip = addNeighborsByCC(edgeSpatialData, currentNeighbors, edgeQueue, visited, currentEdge);
          if (!edgeQueue.empty()) {
            currentEdge = edgeQueue.front();
            edgeQueue.pop_front();
            while (visited[currentEdge] && !edgeQueue.empty()) {
              currentEdge = edgeQueue.front();
              edgeQueue.pop_front();
            }   
          }
          currentNeighbors = edgeNeighbors[currentEdge];
          edgeNeighbors.erase(currentEdge);
        } 
      }
    }
  }
  
  void stripReordering(Set& edgeSet, vector<int>& edgeReordering) {
    int* endpoints = edgeSet.getEndpointsPtr();
    int size = edgeSet.getSize();
    int cardinality = edgeSet.getCardinality();
    
    map<int,vector<int>> vertexToEdgeMap;
    for (int i=0; i < size; ++i) {
      for (int j=0; j < cardinality; ++j) {
        int vertex = endpoints[i*cardinality + j];
        if (vertexToEdgeMap.find(vertex) == vertexToEdgeMap.end()){
          vector<int> edges;
          vertexToEdgeMap[vertex] = edges;
        }
      }
    }
    
    for (int i=0; i < size; ++i) {
      for (int j=0; j < cardinality; ++j) {
        int vertex = endpoints[i*cardinality + j];
        vertexToEdgeMap[vertex].push_back(i);
      }
    }
    
    // Build map of edge id's to vector of neighbor id's
    // Neighbor is another edge that shares 2 or more vertices
    map<int, vector<int>> edgeNeighbors;
    for (int i=0; i < size; ++i) {
      vector<int> neighbors;
      map<int,int> sharedVertexCounts;
      for (int j=0; j < cardinality; ++j) {
        // Iterate through the edges that share a vertex with current edge
        for (auto& connectedEdge : vertexToEdgeMap[endpoints[i*cardinality+j]]) {
          if (i != connectedEdge && sharedVertexCounts.find(connectedEdge) != sharedVertexCounts.end()) {
            sharedVertexCounts[connectedEdge]++; 
          } else {
            sharedVertexCounts[connectedEdge] = 1;
          }
        }
      }
      
      // Add edges to neighbor list if they share more than 2 verts.
      for (auto& edge : sharedVertexCounts) {
        if (edge.second >= 2) {
          neighbors.push_back(edge.first);
        }
      }
      edgeNeighbors[i] = neighbors;
    }
   
    greedyStripReordering(edgeSet, edgeReordering, edgeNeighbors);
  }

  // ---------- Ordering Evaluation Heuristics ----------
  int scoreEdges(const int* endpoints, const int size, const int cardinality) {
    unordered_map<long,vector<long>> distances;
    unordered_map<long,long> firstOccurrence;

    for (int edgeIndex=0; edgeIndex < size * cardinality; edgeIndex += cardinality) {
      for (int elementIndex=0; elementIndex < cardinality; ++elementIndex) {
        auto search = firstOccurrence.find(endpoints[edgeIndex + elementIndex]);
        if ( search != firstOccurrence.end() ) {
          distances[endpoints[edgeIndex + elementIndex]].push_back( edgeIndex - search->second );
        } else {
          firstOccurrence[endpoints[edgeIndex + elementIndex]] = edgeIndex;
          distances[endpoints[edgeIndex + elementIndex]] = vector<long>();
        }
      }
    }

    long distanceSum = 0;
    for ( auto& distMapping : distances ) {
      int distCount = distMapping.second.size();
      for (int offset=0; offset < distCount - 1; ++offset) {
        for (int first=offset; first < distCount - 1; ++first) {
          for (int second=first + 1; second < distCount; ++second) {
            iassert( first < second );
            iassert( distMapping.second[first] < distMapping.second[second]);
            distanceSum += distMapping.second[second] - distMapping.second[first];
          }
        }
      }
    }
    
    return distanceSum;
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
    iassert(vertexOrdering.size() == 0);
    
    // Get new vertex ordering based on given heuristic 
    hilbert::hilbertReorder(vertexSet, vertexOrdering);
    reorderVertexSet(edgeSet, vertexSet, vertexOrdering);

    // Get new edge ordering based on given heuristic 
    // edgeVertexSortReordering(edgeSet.getEndpointsPtr(), edgeOrdering, edgeSet.getSize(), edgeSet.getCardinality());
    stripReordering(edgeSet, edgeOrdering);
    reorderEdgeSet(edgeSet, edgeOrdering);
  }
  
  void reorder(Set& edgeSet, Set& vertexSet) {
    vector<int> vertexOrdering;
    vector<int> edgeOrdering;
    reorder(edgeSet, vertexSet, edgeOrdering, vertexOrdering);
  }
}

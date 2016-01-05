
#include "reorder.h"

#include "graph.h"
#include "hilbert.h"
#include <algorithm>
#include <map>
#include <vector>
#include <list>
#include <unordered_map>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <cfloat>
#include <string>
#include <algorithm>

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

    void bfs(vertex_t * const nodes, const int cntNodes, const vid_t source) {
      for (int i = 0; i < cntNodes; i++) {
        nodes[i].hilbertId = 0;
      }

      vid_t *bfs_q = new (std::nothrow) vid_t[cntNodes];
      bfs_q[0] = source;
      vid_t cur_q_ptr = 0;
      nodes[source].hilbertId = 1;
      vid_t cur_dist = 2;
      vid_t next_batch_start_index = 1;
      vid_t append_q_ptr = 1;

      bool done = false;
      while (!done) {
        done = true;
        while (cur_q_ptr != next_batch_start_index) {
          vid_t vid = bfs_q[cur_q_ptr];
          vid_t *edges = nodes[vid].edgeData.edges;
          for (vid_t e = 0; e < nodes[vid].edgeData.cntEdges; e++) {
            if (nodes[edges[e]].hilbertId == 0) {
              nodes[edges[e]].hilbertId = cur_dist;
              bfs_q[append_q_ptr++] = edges[e];
            }
          }
          cur_q_ptr++;
        }
        // we added more nodes to the queue
        if (next_batch_start_index < append_q_ptr) {
          done = false;
        }
        // start of the next distance (from the BFS root) in the BFS queue
        next_batch_start_index = append_q_ptr;
        cur_dist++;
      }

      delete bfs_q;
    }

    void assignBfsIds(vertex_t * const nodes, const int cntNodes) {
      bfs(nodes, cntNodes, 0);
    }

    void assignRandomIds(vertex_t * const nodes, const int cntNodes) {
      vector<int> ordering;
      for (int i = 0; i < cntNodes; i++) {
        ordering.push_back(i);
      }
      random_shuffle(ordering.begin(), ordering.end());
      for (int i = 0; i < cntNodes; i++) {
        nodes[i].hilbertId = ordering[i];
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
      vertex_t * nodes = new (std::nothrow) vertex_t[numNodes];
      
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

    void hilbertReorder(Set& vertexSet, vector<int>& vertexOrdering, int cntNodes) {
      vertex_t * nodes;
      const int hilbertBits = 8;  
      loadNodes(vertexSet, &nodes, cntNodes);

      //assignBfsIds(nodes, cntNodes);
      //assignRandomIds(nodes, cntNodes);
      assignHilbertIds(nodes, cntNodes, hilbertBits);

      stable_sort(nodes, nodes + cntNodes, vertexComparator);

      createIdTranslationMapping(nodes, vertexOrdering, cntNodes);

    }
  } // namespace simit::hilbert
 
  // ---------- Simit Level Reordering Heuristics ----------
  void bfs(vector<int>& vertexOrdering, int* endpoints, int size, int cardinality)
  {
    list<int> queue;
    vertexOrdering.resize(size);
    vector<bool> visited(size);
    iassert(visited.size() == size);
    iassert(vertexOrdering.size() == size);
    
    for (int i=0; i < size; i++) 
    {
      visited[i] = false;
    }
    
    // Element id of the starting vertex
    int vertex = 0; 
    int counter = 0; 
    int endpoint;
    
    // Add the starting vertex to the queue
    visited[vertex] = true;
    queue.push_back(vertex);
    vertexOrdering[vertex] = counter++;
    iassert(vertexOrdering[vertex] < size);
    iassert(vertexOrdering[vertex] == 0);

    while (!queue.empty()) 
    {
      vertex = queue.front();
      iassert(visited[vertex]);
      queue.pop_front();

      for (int i=0; i<cardinality; ++i) {
        endpoint = endpoints[vertex * cardinality + i];
        
        if (!visited[endpoint])
        {
          visited[endpoint] = true;
          queue.push_back(endpoint);
          vertexOrdering[endpoint] = counter++;
          iassert(vertexOrdering[endpoint] < size);
        }
      }
    }

    for (int vertex=0; vertex < size; ++vertex) {
      if (!visited[vertex])
      {
        visited[vertex] = true;
        vertexOrdering[vertex] = counter++;
        iassert(vertexOrdering[vertex] < size);
        for (int i=0; i<cardinality; ++i) {
          endpoint = endpoints[vertex * cardinality + i];
          if (!visited[endpoint])
          {
            visited[endpoint] = true;
            vertexOrdering[endpoint] = counter++;
            iassert(vertexOrdering[endpoint] < size)
                << vertexOrdering[endpoint] << " < " << size;
          }
        }
      } 
    }
  }

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

    int sum;
    int edgeIndex; 
    for (int i=0; i < size; ++i) {
      edgeIndex = i * cardinality;
      sum = 0;
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
  
  // ---------- Ordering Evaluation Heuristics ----------
  int scoreEdges(const int* endpoints, const int size, const int cardinality) {
    unordered_map<int,vector<int>> distances;
    unordered_map<int,int> firstOccurrence;

    for (int edgeIndex=0; edgeIndex < size * cardinality; edgeIndex += cardinality) {
      for (int elementIndex=0; elementIndex < cardinality; ++elementIndex) {
        auto search = firstOccurrence.find(endpoints[edgeIndex + elementIndex]);
        if ( search != firstOccurrence.end() ) {
          distances[endpoints[edgeIndex + elementIndex]].push_back( edgeIndex - search->second );
        } else {
          firstOccurrence[endpoints[edgeIndex + elementIndex]] = edgeIndex;
          distances[endpoints[edgeIndex + elementIndex]] = vector<int>();
        }
      }
    }

    int distanceSum = 0;
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

    for (int edgeIndex=0; edgeIndex< size; ++edgeIndex) {
      iassert(edgeIndex < edgeOrdering.size());
      iassert(edgeOrdering[edgeIndex] < (size - 1) * cardinality * sizeof(int));
      memcpy(newEndpoints + edgeIndex * cardinality, endpoints + edgeOrdering[edgeIndex] * cardinality, cardinality * sizeof(int));
    }
    memcpy(endpoints, newEndpoints, size * cardinality * sizeof(int));
    delete newEndpoints;

    assert(size == edgeOrdering.size());
    
    reorderFields(edgeSet.getFields(), edgeOrdering);
  }

  void reorderVertexSet(Set& edgeSet, Set& vertexSet, vector<int>& vertexOrdering) {
    // Reset Endpoints to reflect reordering
    //    Vertex ordering maps old to new identity 
    //    This itertates over all enpoints 'translating' old to new
    for (int i=0; i < edgeSet.getSize() * edgeSet.getCardinality(); ++i) {
      iassert(vertexOrdering[edgeSet.getEndpointsPtr()[i]] < vertexSet.getSize());
      edgeSet.getEndpointsPtr()[i] = vertexOrdering[edgeSet.getEndpointsPtr()[i]]; 
    }
    
    reorderFields(vertexSet.getFields(), vertexOrdering);
  }
  
  void reorder(Set& edgeSet, Set& vertexSet, vector<int>& vertexOrdering) {
    iassert(vertexOrdering.size() == 0);
    
    // Get new vertex ordering based on given heuristic 
    // bfs(vertexOrdering, edgeSet.getEndpointsPtr(), vertexSet.getSize(), edgeSet.getCardinality());
    hilbert::hilbertReorder(vertexSet, vertexOrdering, vertexSet.getSize());
    iassert(vertexOrdering.size() == vertexSet.getSize()) << vertexOrdering.size() << ", " << vertexSet.getSize();
    reorderVertexSet(edgeSet, vertexSet, vertexOrdering);

    // vector<int> edgeOrdering;
    // edgeSumReordering(edgeSet.getEndpointsPtr(), edgeOrdering, edgeSet.getSize(), edgeSet.getCardinality());
    // iassert(edgeOrdering.size() == edgeSet.getSize()); 
    // reorderEdgeSet(edgeSet, edgeOrdering);
  }

}

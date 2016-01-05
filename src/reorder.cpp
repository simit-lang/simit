
#include "reorder.h"

#include "graph.h"
#include "hilbert-reorder.h"
#include <algorithm>
#include <map>
#include <vector>
#include <list>
#include <unordered_map>

using namespace std;
namespace simit {
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
  
  void reorderEdges(int* endpoints, const vector<int>& edgeOrdering, const int size, const int cardinality) {
    int* newEndpoints = static_cast<int *>(malloc(size * cardinality * sizeof(int)));
    memcpy(newEndpoints, endpoints, size * cardinality * sizeof(int));

    for (int edgeIndex=0; edgeIndex< size; ++edgeIndex) {
      iassert(edgeIndex < edgeOrdering.size());
      iassert(edgeOrdering[edgeIndex] < (size - 1) * cardinality * sizeof(int));
      memcpy(newEndpoints + edgeIndex * cardinality, endpoints + edgeOrdering[edgeIndex] * cardinality, cardinality * sizeof(int));
    }
    memcpy(endpoints, newEndpoints, size * cardinality * sizeof(int));
    delete newEndpoints;
  }

  void reorder(Set& edgeSet, Set& vertexSet, vector<int>& vertexOrdering) {
    
    //cout << "Score Pre Reorder" << endl;
    //cout << "Total edge distance: " << scoreEdges(edgeSet.getEndpointsPtr(), getSize(), getCardinality()) << endl;
    
    // Get new vertex ordering based on given heuristic 
    iassert(vertexOrdering.size() == 0);
    //bfs(vertexOrdering, edgeSet.getEndpointsPtr(), vertexSet.getSize(), edgeSet.getCardinality());
    hilbertReorder(vertexSet, vertexOrdering, vertexSet.getSize());
    
    //vertexDegreeReordering(vertexOrdering, edgeSet.getEndpointsPtr(), getSize(), getCardinality());
    
    iassert(vertexOrdering.size() == vertexSet.getSize()) << vertexOrdering.size() << ", " << vertexSet.getSize();
    
    // Reset Endpoints to reflect reordering
    //    Vertex ordering maps old to new identity 
    //    This itertates over all enpoints 'translating' old to new
    for (int i=0; i < edgeSet.getSize() * edgeSet.getCardinality(); ++i) {
      iassert(vertexOrdering[edgeSet.getEndpointsPtr()[i]] < vertexSet.getSize());
      edgeSet.getEndpointsPtr()[i] = vertexOrdering[edgeSet.getEndpointsPtr()[i]]; 
    }
    
    for (auto f : vertexSet.getFields()) {
      switch (f->type->getComponentType()) {
        case ComponentType::Float: {
          float* data = static_cast<float *>(f->data);
          reorderFieldData(data, vertexOrdering, vertexSet.getSize(), f->sizeOfType);
          break;
        }
        case ComponentType::Double: {
          double* data = static_cast<double *>(f->data);
          reorderFieldData(data, vertexOrdering, vertexSet.getSize(), f->sizeOfType);
          break;
        }
        case ComponentType::Int: {
          int* data = static_cast<int *>(f->data);
          reorderFieldData(data, vertexOrdering, vertexSet.getSize(), f->sizeOfType);
          break;
        }
        case ComponentType::Boolean: {
          bool* data = static_cast<bool *>(f->data);
          reorderFieldData(data, vertexOrdering, vertexSet.getSize(), f->sizeOfType);
        }
      }
    }
    
    // vector<int> edgeOrdering;
    // //cout << "Score before : " << scoreEdges(edgeSet.getEndpointsPtr(),edgeSet.getSize(), edgeSet.getCardinality()) << endl;
    // edgeSumReordering(edgeSet.getEndpointsPtr(), edgeOrdering, edgeSet.getSize(), edgeSet.getCardinality());
    // iassert(edgeOrdering.size() == edgeSet.getSize()); 
    // reorderEdges(edgeSet.getEndpointsPtr(), edgeOrdering, edgeSet.getSize(), edgeSet.getCardinality()); 
    // //cout << "Score after : " << scoreEdges(edgeSet.getEndpointsPtr(),edgeSet.getSize(), edgeSet.getCardinality()) << endl;
    // 
    // for (auto f : edgeSet.getFields()) {
    //   switch (f->type->getComponentType()) {
    //     case ComponentType::Float: {
    //       float* data = static_cast<float *>(f->data);
    //       reorderFieldData(data, edgeOrdering, edgeSet.getSize(), f->sizeOfType);
    //       break;
    //     }
    //     case ComponentType::Double: {
    //       double* data = static_cast<double *>(f->data);
    //       reorderFieldData(data, edgeOrdering, edgeSet.getSize(), f->sizeOfType);
    //       break;
    //     }
    //     case ComponentType::Int: {
    //       int* data = static_cast<int *>(f->data);
    //       reorderFieldData(data, edgeOrdering, edgeSet.getSize(), f->sizeOfType);
    //       break;
    //     }
    //     case ComponentType::Boolean: {
    //       bool* data = static_cast<bool *>(f->data);
    //       reorderFieldData(data, edgeOrdering, edgeSet.getSize(), f->sizeOfType);
    //     }
    //   }
    // }
  }
}

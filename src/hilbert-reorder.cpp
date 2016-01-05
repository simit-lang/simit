#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <cfloat>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include "hilbert.h"
#include "hilbert-reorder.h"

using namespace std;

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

  WHEN_DEBUG({
    printf("Coordinate extremes:\n");
    printf("xMin -- xMax: %.3f -- %.3f\n", xMin, xMax);
    printf("yMin -- yMax: %.3f -- %.3f\n", yMin, yMax);
    printf("zMin -- zMax: %.3f -- %.3f\n", zMin, zMax);
    printf("\n");
  })

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

    WHEN_DEBUG({
      double coords[3];
      coords[0] = (nodes[i].x - xMin) * (hilbertGridN - 1) / xMax;
      coords[1] = (nodes[i].y - yMin) * (hilbertGridN - 1) / yMax;
      coords[2] = (nodes[i].z - zMin) * (hilbertGridN - 1) / zMax;

      printf("hilbert data for node id %lu:\n", nodes[i].id);
      printf("  rescale X: %.3f\n", coords[0]);
      printf("  rescale Y: %.3f\n", coords[1]);
      printf("  rescale Z: %.3f\n", coords[2]);
      printf("  lattice X: %lld\n", latticeCoords[0]);
      printf("  lattice Y: %lld\n", latticeCoords[1]);
      printf("  lattice Z: %lld\n", latticeCoords[2]);
      printf("  hilbertId: %lld\n", hilbertIndex);
      printf("\n");
    })
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

namespace simit {
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
}

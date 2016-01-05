#ifndef HILBERT_REORDER_H_
#define HILBERT_REORDER_H_

#include <vector>
#include <cinttypes>
#include <cassert>
#include <utility>
#include <graph.h>

#ifndef TEST
#define TEST 0
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

// Use WHEN_TEST to conditionally include expensive sanity-checks,
// when doing more work than simply checking an assertion. For example:
//
// uint64_t result = fast_computation_method();
// WHEN_TEST({
//   uint64_t expectedResult = slow_but_safe_computation_method();
//   assert(result == expectedResult);
// })
#if TEST
  #define WHEN_TEST(ex) { ex }
#else
  #define WHEN_TEST(ex)
#endif

// Always wrap your debugging code in WHEN_DEBUG, so it doesn't
// get included into the executable by accident:
// WHEN_DEBUG({
//   printf("Debugging...");
//   printf("Still debugging...");
// })
#if DEBUG
  #define WHEN_DEBUG(ex) { ex }
#else
  #define WHEN_DEBUG(ex)
#endif

#ifndef HILBERTBITS
  #define HILBERTBITS 4
#endif

#ifndef PARALLEL
  #define PARALLEL 1
#endif

#if PARALLEL
  #include <omp.h>
#endif

#ifndef BFS
  #define BFS 0
#endif

#ifndef VID_DECLARED_
#define VID_DECLARED_
typedef uint64_t vid_t;  // vertex id type
#endif

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

namespace simit {
  void hilbertReorder(Set& vertexSet, std::vector<int>& vertexOrdering, int vertexCount);
}
#endif  // HILBERT_REORDER_H 

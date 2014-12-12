#include "indices.h"

namespace simit {
namespace internal {

// class VertexToEdgeEndpointIndex
VertexToEdgeEndpointIndex:: VertexToEdgeEndpointIndex(const SetBase &edgeSet) {
  totalEdges = edgeSet.getSize();
  for (int i=0; i<edgeSet.getCardinality(); ++i) {
    auto es = edgeSet.getEndpointSet(i);
    endpointSets.push_back(es);

    // allocate array to contain how many edges each element is part of
    // calloc sets them all to zero
    numEdgesForVertex.push_back((int*)calloc(sizeof(int),es->getSize()));

    // allocate array to contain which edges each element is part of
    whichEdgesForVertex.push_back((int**)calloc(sizeof(int*),es->getSize()));
    for (int i=0; i<es->getSize(); i++) {
      whichEdgesForVertex[endpointSets.size()-1][i] =
          (int*)malloc(sizeof(int)* totalEdges);
    }
  }

  for (auto e : edgeSet) {
    for (int epi=0; epi<(int)(endpointSets.size()); epi++) {
      auto ep = edgeSet.getEndpoint(e, epi);
      whichEdgesForVertex[epi][ep.ident][numEdgesForVertex[epi][ep.ident]++] =
      e.ident;
    }
  }
}

VertexToEdgeEndpointIndex::~VertexToEdgeEndpointIndex() {
  for (auto ne : numEdgesForVertex)
    free(ne);
  for (int w=0; w<(int)(endpointSets.size()); w++) {
    for (int i=0; i<endpointSets[w]->getSize(); i++)
      free(whichEdgesForVertex[w][i]);
    free(whichEdgesForVertex[w]);
  }
}


// class VertexToEdgeIndex
VertexToEdgeIndex::VertexToEdgeIndex(const SetBase &edgeSet) {
  totalEdges = edgeSet.getSize();
  for (int i=0; i<edgeSet.getCardinality(); ++i) {
    auto es = edgeSet.getEndpointSet(i);

    endpointSets.push_back(es);

    // if we already have this set in the container, don't insert again.
    // this case occurs if the edgeset is homogenous, for example.
    if (numEdgesForVertex.count(es) > 0)
      continue;

    // allocate array to contain how many edges each element is part of
    // calloc sets them all to zero
    numEdgesForVertex[es] = (int*)calloc(sizeof(int), es->getSize());

    // allocate array to contain which edges each element is part of
    whichEdgesForVertex[es] = (int**)calloc(sizeof(int*), es->getSize());
    for (int i=0; i<es->getSize(); i++) {
      whichEdgesForVertex[es][i] = (int*)malloc(sizeof(int)* totalEdges);
    }
  }

  for (auto e : edgeSet) {
    for (int epi=0; epi<(int)(endpointSets.size()); epi++) {
      auto ep = edgeSet.getEndpoint(e, epi);
      whichEdgesForVertex[endpointSets[epi]]
      [ep.ident][numEdgesForVertex[endpointSets[epi]][ep.ident]++] =
      e.ident;
    }
  }
}

VertexToEdgeIndex::~VertexToEdgeIndex() {
  for (auto ne : numEdgesForVertex) {
    free(ne.second);
  }
  for (auto s : whichEdgesForVertex) {
    for (int i=0; i<s.first->getSize(); i++)
      free(s.second[i]);
    free(s.second);
  }
}


// class NeighborIndex
NeighborIndex::NeighborIndex(const SetBase &edgeSet) {
  VertexToEdgeIndex VToE(edgeSet);

  //number of vertices per edge
  unsigned cardinality = edgeSet.getCardinality();

  std::vector<std::vector<int>> edgeToVertex(edgeSet.getSize());
  for(auto e : edgeSet){
    edgeToVertex[e.ident].resize(cardinality, 0);
    for (unsigned epi=0; epi<cardinality; epi++) {
      auto ep = edgeSet.getEndpoint(e, epi);
      edgeToVertex[e.ident][epi] = ep.ident;
    }
  }

  const SetBase* vSet = edgeSet.getEndpointSet(0);
  startIndex = (int*)malloc(sizeof(int) * vSet->getSize());
  startIndex[0] = 0;
  for(auto v : *vSet){
    int * edgeNeighbors = VToE.getWhichEdgesForElement(v, *vSet);
    int   nEdgeNeighbor = VToE.getNumEdgesForElement  (v, *vSet);
    std::vector<int> nbr;
    for(int ii = 0; ii<nEdgeNeighbor; ii++){
      int eIdx = edgeNeighbors[ii];
      for(unsigned jj = 0; jj<cardinality; jj++){
        int nbrIdx = edgeToVertex[eIdx][jj];
        addNoCollision(nbrIdx, nbr);
      }
    }
    neighbors.insert(neighbors.end(), nbr.begin(), nbr.end());
    startIndex[v.ident+1] = neighbors.size();
  }
}

NeighborIndex::~NeighborIndex() {
  free(startIndex);
}

void NeighborIndex::addNoCollision(int x, std::vector<int> & a) {
  for(unsigned int ii=0 ;ii<a.size();ii++){
    if(a[ii]==x){
      return;
    }
  }
  a.push_back(x);
}

}}

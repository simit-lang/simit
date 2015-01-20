#include "indices.h"

namespace simit {
namespace internal {

// class VertexToEdgeEndpointIndex
VertexToEdgeEndpointIndex:: VertexToEdgeEndpointIndex(const SetBase &edgeSet) {
  totalEdges = edgeSet.getSize();
  for (int i=0; i<edgeSet.getCardinality(); ++i) {
    auto es = edgeSet.getEndpointSet(i);
    endpointSets.push_back(es);
  }

  for (auto e : edgeSet) {
    for (int epi=0; epi<(int)(endpointSets.size()); epi++) {
      auto ep = edgeSet.getEndpoint(e, epi);
      whichEdgesForVertex[std::make_pair(epi, ep.ident)].insert(e.ident);
    }
  }
}

VertexToEdgeEndpointIndex::~VertexToEdgeEndpointIndex() {
}


// class VertexToEdgeIndex
VertexToEdgeIndex::VertexToEdgeIndex(const SetBase &edgeSet) {
  totalEdges = edgeSet.getSize();
  for (int i=0; i<edgeSet.getCardinality(); ++i) {
    auto es = edgeSet.getEndpointSet(i);

    endpointSets.push_back(es);
  }

  for (auto e : edgeSet) {
    for (int epi=0; epi<(int)(endpointSets.size()); epi++) {
      auto ep = edgeSet.getEndpoint(e, epi);
      whichEdgesForVertex[std::make_pair(
          endpointSets[epi], ep.ident)].insert(e.ident);
    }
  }
}

VertexToEdgeIndex::~VertexToEdgeIndex() {
}


// class NeighborIndex
NeighborIndex::NeighborIndex(const SetBase &edgeSet) {
  VertexToEdgeIndex VToE(edgeSet);

  //number of vertices per edge
  unsigned cardinality = edgeSet.getCardinality();

  const SetBase* vSet = edgeSet.getEndpointSet(0);
  startIndex = (int*)malloc(sizeof(int) * (vSet->getSize()+1));
  startIndex[0] = 0;
  for(auto v : *vSet){
    std::set<int> edgeNeighbors = VToE.getWhichEdgesForElement(v, *vSet);

    std::vector<int> nbr;
    for(int eIdx : edgeNeighbors) {
      for(unsigned jj = 0; jj<cardinality; jj++){
        int nbrIdx = edgeSet.getEndpoint(ElementRef(eIdx), jj).ident;
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

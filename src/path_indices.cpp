#include "path_indices.h"

#include <iostream>
#include "path_expressions.h"
#include "graph.h"

namespace simit {
namespace pe {

// class PathIndex
std::ostream &operator<<(std::ostream &os, const PathIndex &pi) {
  if (pi.ptr != nullptr) {
    os << *pi.ptr;
  }
  else {
    os << "empty path idex";
  }
  return os;
}

// class SetEndpointPathIndex
SetEndpointPathIndex::SetEndpointPathIndex(const Set &edgeSet)
    : edgeSet{edgeSet} {
  // TODO: Generalize to support gaps in the future
  iassert(edgeSet.isHomogeneous())
      << "Must be homogeneous because otherwise there are gaps";
}

unsigned SetEndpointPathIndex::numElements() const {
    return edgeSet.getSize();
}

unsigned SetEndpointPathIndex::numNeighbors(unsigned elemID) const {
  return edgeSet.getCardinality();
}

unsigned SetEndpointPathIndex::numNeighbors() const {
  return numElements() * edgeSet.getCardinality();
}

SetEndpointPathIndex::Neighbors
SetEndpointPathIndex::neighbors(unsigned elemID) const {
  class SetEndpointNeighbors : public PathIndexImpl::Neighbors::Base {
    class Iterator : public PathIndexImpl::Neighbors::Iterator::Base {
    public:
      Iterator(const Set::Endpoints::Iterator &epit) : epit(epit) {}

      void operator++() {++epit;}
      unsigned operator*() const {return epit->getIdent();}
      Base* clone() const {return new Iterator(*this);}

    protected:
      bool equal(const Base& o) const {
        const Iterator *other = static_cast<const Iterator*>(&o);
        return epit == other->epit;
      }

    private:
      Set::Endpoints::Iterator epit;
    };

  public:
    SetEndpointNeighbors(const Set::Endpoints &endpoints)
        : endpoints(endpoints) {}

    Neighbors::Iterator begin() const {return new Iterator(endpoints.begin());}
    Neighbors::Iterator end() const {return new Iterator(endpoints.end());}

  private:
    Set::Endpoints endpoints;
  };

  return new SetEndpointNeighbors(edgeSet.getEndpoints(ElementRef(elemID)));
}

void SetEndpointPathIndex::print(std::ostream &os) const {
  os << "SetEndpointPathIndex:";
  for (auto &e : *this) {
    os << "\n" << "  " << e << ": ";
    for (auto ep : neighbors(e)) {
      os << ep << " ";
    }
  }
}

// class SegmentedPathIndex
SegmentedPathIndex::Neighbors
SegmentedPathIndex::neighbors(unsigned elemID) const {
  class SegmentNeighbors : public PathIndexImpl::Neighbors::Base {
    class Iterator : public PathIndexImpl::Neighbors::Iterator::Base {
    public:
      /// nbrs points to the neighbor segment of the current element.
      Iterator(unsigned currNbr, const unsigned *nbrs)
          : currNbr(currNbr), nbrs(nbrs) {}

      void operator++() {++currNbr;}
      unsigned operator*() const {return nbrs[currNbr];}
      Base* clone() const {return new Iterator(*this);}

    protected:
      bool equal(const Base& o) const {
        const Iterator *other = static_cast<const Iterator*>(&o);
        return currNbr == other->currNbr;
      }

    private:
      unsigned currNbr;
      const unsigned *nbrs;
    };

  public:
    SegmentNeighbors(unsigned numNbrs, const unsigned *nbrs)
        : numNbrs(numNbrs), nbrs(nbrs) {}

    Neighbors::Iterator begin() const {return new Iterator(0, nbrs);}
    Neighbors::Iterator end() const {return new Iterator(numNbrs, nbrs);}

  private:
    unsigned numNbrs;
    const unsigned *nbrs;
  };

  return new SegmentNeighbors(numNeighbors(elemID), &nbrs[nbrsStart[elemID]]);
}

void SegmentedPathIndex::print(std::ostream &os) const {
  os << "SegmentedPathIndex:";
  std::cout << std::endl;
  for (size_t i=0; i < numElements()+1; ++i) {
    std::cout << nbrsStart[i] << " ";
  }
  std::cout << std::endl;
  for (size_t i=0; i < numNeighbors(); ++i) {
    std::cout << nbrs[i] << " ";
  }
}


// class PathIndexBuilder
PathIndex PathIndexBuilder::buildSegmented(const PathExpression &pe,
                                     unsigned sourceEndpoint,
                                     std::map<ElementVar,const Set&> bindings) {
  // Check if we have memoized the path index for this path expression, starting
  // at this sourceEndpoint, bound to these sets.
  // TODO

  // Interpret the path expression, starting at sourceEndpoint, over the graph.
  // That is given an element, the find its neighbors through the paths
  // described by the path expression.
  class PathNeighborVisitor : public PathExpressionVisitor {
  public:
    PathNeighborVisitor(const std::map<ElementVar,const Set&> &bindings)
        : bindings(bindings) {}

    PathIndex build(const PathExpression &pe) {
      pe.accept(this);
      PathIndex pit = pi;
      pi = nullptr;
      return pit;
    }

  private:
    /// If it is an EV path expression we return an EndpointPathIndex that wraps
    /// the Edge set.
    void visit(const EV *ev) {
      const Set &edgeSet = bindings.at(ev->getPathEndpoint(0));
      iassert(edgeSet.getCardinality() > 0) << "not an edge set";
      pi = new SetEndpointPathIndex(edgeSet);
    };

    void visit(const VE *ve) {
      const Set &edgeSet = bindings.at(ve->getPathEndpoint(1));
      iassert(edgeSet.getCardinality() > 0) << "not an edge set";
      std::map<unsigned,std::vector<unsigned>> neighbors;
      for (auto &e : edgeSet) {
        iassert(e.getIdent() >= 0);
        for (auto &ep : edgeSet.getEndpoints(e)) {
          iassert(ep.getIdent() >= 0);
          neighbors[ep.getIdent()].push_back(e.getIdent());
        }
      }

      unsigned numNeighbors = 0;
      for (auto &p : neighbors) {
        numNeighbors += p.second.size();
      }

      unsigned numElements = neighbors.size();
      unsigned *nbrsStart = new unsigned[numElements + 1];
      unsigned *nbrs = new unsigned[numNeighbors];

      int currElem = 0;
      int currNbrsStart = 0;
      for (auto &p : neighbors) {
        nbrsStart[p.first] = currNbrsStart;
        memcpy(&nbrs[currNbrsStart], p.second.data(),
               p.second.size() * sizeof(unsigned));
        currElem += 1;
        currNbrsStart += p.second.size();
      }
      nbrsStart[numElements] = currNbrsStart;

      pi = new SegmentedPathIndex(numElements, nbrsStart, nbrs);
    }

    void visit(const Predicate *p) {
      pi = new SegmentedPathIndex();
    }

    PathIndex pi;
    const std::map<ElementVar,const Set&> &bindings;
  };

  return PathNeighborVisitor(bindings).build(pe);
}

}}

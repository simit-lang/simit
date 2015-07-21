#include "path_indices.h"

#include <iostream>
#include <stack>
#include <map>
#include <vector>

#include "path_expressions.h"
#include "graph.h"

using namespace std;

namespace simit {
namespace pe {

// class PathIndex
std::ostream &operator<<(std::ostream &os, const PathIndex &pi) {
  if (pi.ptr != nullptr) {
    os << *pi.ptr;
  }
  else {
    os << "empty PathIndex";
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
      bool eq(const Base& o) const {
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
      bool eq(const Base& o) const {
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
  os << "\n  ";
  for (size_t i=0; i < numElements()+1; ++i) {
    os << nbrsStart[i] << " ";
  }
  os << "\n  ";
  for (size_t i=0; i < numNeighbors(); ++i) {
    os << nbrs[i] << " ";
  }
}


// class PathIndexBuilder
PathIndex PathIndexBuilder::buildSegmented(const PathExpression &pe,
                                           unsigned sourceEndpoint){
  iassert(pe.isBound())
      << "attempting to build an index from a path expression (" << pe
      << ") that is not bound to sets";

  // Interpret the path expression, starting at sourceEndpoint, over the graph.
  // That is given an element, the find its neighbors through the paths
  // described by the path expression.
  class PathNeighborVisitor : public PathExpressionVisitor {
  public:
    PathNeighborVisitor(PathIndexBuilder *builder) : builder(builder) {}

    PathIndex build(const PathExpression &pe) {
      pe.accept(this);
      PathIndex pit = pi;
      pi = nullptr;
      return pit;
    }

  private:
    /// Pack neighbor vectors into a segmented vector (contiguous array).
    PathIndex pack(const map<unsigned, set<unsigned>> &pathNeighbors) {
      unsigned numNeighbors = 0;
      for (auto &p : pathNeighbors) {
        numNeighbors += p.second.size();
      }

      unsigned numElements = pathNeighbors.size();
      unsigned *nbrsStart = new unsigned[numElements + 1];
      unsigned *nbrs = new unsigned[numNeighbors];

      int currNbrsStart = 0;
      for (auto &p : pathNeighbors) {
        unsigned elem = p.first;

        vector<unsigned> pNeighbors(p.second.begin(), p.second.end());
        sort(pNeighbors.begin(), pNeighbors.end());

        nbrsStart[elem] = currNbrsStart;
        memcpy(&nbrs[currNbrsStart], pNeighbors.data(),
               pNeighbors.size() * sizeof(unsigned));

        currNbrsStart += pNeighbors.size();
      }
      nbrsStart[numElements] = currNbrsStart;
      return new SegmentedPathIndex(numElements, nbrsStart, nbrs);;
    }

    /// If it is an EV path expression we return an EndpointPathIndex that wraps
    /// the Edge set.
    void visit(const EV *ev) {
      const Set &edgeSet = *ev->getE().getBinding();
      iassert(edgeSet.getCardinality() > 0)
          << "not an edge set" << edgeSet.getName();
      pi = new SetEndpointPathIndex(edgeSet);
    };

    void visit(const VE *ve) {
      const Set &edgeSet = *ve->getE().getBinding();
      iassert(edgeSet.getCardinality() > 0)
          << "not an edge set" << edgeSet.getName();

      // Add each edge to the neighbor vectors of its endpoints
      map<unsigned, set<unsigned>> pathNeighbors;
      for (auto &e : edgeSet) {
        iassert(e.getIdent() >= 0);
        for (auto &ep : edgeSet.getEndpoints(e)) {
          iassert(ep.getIdent() >= 0);
          pathNeighbors[ep.getIdent()].insert(e.getIdent());
        }
      }

      pi = pack(pathNeighbors);
    }

    void visit(const QuantifiedAnd *f) {
      auto &freeVars = f->getFreeVars();
      iassert(freeVars.size() == 2)
          << "For now, we only support matrix path expressions";

      PathExpression lhs = f->getLhs();
      PathExpression rhs = f->getRhs();

      map<unsigned, set<unsigned>> pathNeighbors;
      if (f->isQuantified()) {
        iassert(f->getQVars().size() == 1)
            << "For now, we only support one quantified variable";

        QVar qvar = f->getQVars()[0];

        // The expression combines two binary path expressions with one
        // quantified variable. Thus, each operand must link one of the two free
        // variables to the quantified variable.

        map<Var, vector<pair<PathExpression,unsigned>>> varToLocations;
        varToLocations[lhs.getPathEndpoint(0)].push_back({lhs,0});
        varToLocations[lhs.getPathEndpoint(1)].push_back({lhs,1});
        varToLocations[rhs.getPathEndpoint(0)].push_back({rhs,0});
        varToLocations[rhs.getPathEndpoint(1)].push_back({rhs,1});

        iassert(varToLocations.find(qvar.getVar()) != varToLocations.end())
            << "could not find quantified variable locations";
        iassert(varToLocations[qvar.getVar()].size() == 2)
            << "quantified binary expr only uses quantified variable once";

        // Build a path index from the first free variable to the quantified
        // variable
        pair<PathExpression,unsigned> sourceLoc= varToLocations[freeVars[0]][0];
        PathIndex sourceToQuantified =
            builder->buildSegmented(sourceLoc.first, sourceLoc.second);

        // Build a path index from the quantified variable to the second free
        // variable
        pair<PathExpression,unsigned>  sinkLoc = varToLocations[freeVars[1]][0];
        unsigned quantifiedLoc = ((sinkLoc.second) == 0) ? 1 : 0;
        PathIndex quantifiedToSink =
            builder->buildSegmented(sinkLoc.first, quantifiedLoc);

        // Build a path index from the first free variable to the second free
        // variable, through the quantified variable.
        for (unsigned source : sourceToQuantified) {
          for (unsigned q : sourceToQuantified.neighbors(source)) {
            for (unsigned sink : quantifiedToSink.neighbors(q)) {
              pathNeighbors[source].insert(sink);
            }
          }
        }
        
      }
      else {
        not_supported_yet;
      }

      pi = pack(pathNeighbors);
    }

    PathIndex pi;  // Path index returned from cases
    PathIndexBuilder *builder;
  };

  // TODO: Possible optimization is to ∂etect symmetric path expressions, and
  //       return the same path index when they are evaluated in both directions

  // Check if we have memoized the path index for this path expression, starting
  // at this sourceEndpoint, bound to these sets.
  if (pathIndices.find({pe,sourceEndpoint}) != pathIndices.end()) {
    return pathIndices.at({pe,sourceEndpoint});
  }

  PathIndex pi = PathNeighborVisitor(this).build(pe);
  pathIndices.insert({{pe,sourceEndpoint}, pi});
  return pi;
}

}}

#include "path_indices.h"

#include <iostream>
#include <stack>
#include <map>
#include <vector>

#include "path_expressions.h"
#include "graph.h"
#include "util/collections.h"

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
SetEndpointPathIndex::SetEndpointPathIndex(const simit::Set &edgeSet)
    : edgeSet(edgeSet) {
  // TODO: Generalize to support gaps in the future
  simit_iassert(edgeSet.isHomogeneous())
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
      Iterator(const simit::Set::Endpoints::Iterator &epit) : epit(epit) {}

      void operator++() {++epit;}
      unsigned operator*() const {return epit->getIdent();}
      Base* clone() const {return new Iterator(*this);}

    protected:
      bool eq(const Base& o) const {
        const Iterator *other = static_cast<const Iterator*>(&o);
        return epit == other->epit;
      }

    private:
      simit::Set::Endpoints::Iterator epit;
    };

  public:
    SetEndpointNeighbors(const simit::Set::Endpoints &endpoints)
        : endpoints(endpoints) {}

    Neighbors::Iterator begin() const {return new Iterator(endpoints.begin());}
    Neighbors::Iterator end() const {return new Iterator(endpoints.end());}

  private:
    simit::Set::Endpoints endpoints;
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

  return new SegmentNeighbors(numNeighbors(elemID),
                              &sinksData[coordsData[elemID]]);
}

void SegmentedPathIndex::print(std::ostream &os) const {
  os << "SegmentedPathIndex:";
  os << "\n  ";
  for (size_t i=0; i < numElements()+1; ++i) {
    os << coordsData[i] << " ";
  }
  os << "\n  ";
  for (size_t i=0; i < numNeighbors(); ++i) {
    os << sinksData[i] << " ";
  }
}


// class PathIndexBuilder
PathIndex PathIndexBuilder::buildSegmented(const PathExpression &pe,
                                           unsigned sourceEndpoint){
  /// Interpret the path expression, starting at sourceEndpoint, over the graph.
  /// That is given an element, the find its neighbors through the paths
  /// described by the path expression.
  class PathNeighborVisitor : public PathExpressionVisitor {
  public:
    struct Location {
      PathExpression pathExpr;
      unsigned endpoint;
    };
    typedef map<Var, vector<Location>> VarToLocationsMap;

    PathNeighborVisitor(PathIndexBuilder *builder) : builder(builder) {}

    PathIndex build(const PathExpression &pe) {
      pe.accept(this);
      PathIndex pit = pi;
      pi = nullptr;
      return pit;
    }

  private:
    /// Pack neighbor vectors into a segmented vector (contiguous array).
    PathIndex pack(const map<unsigned, vector<unsigned>> &pathNeighbors,
                   bool sorted=true) {
      unsigned numNeighbors = 0;
      for (auto &p : pathNeighbors) {
        numNeighbors += p.second.size();
      }

      size_t numElements = pathNeighbors.size();
      uint32_t* coordsData= (uint32_t*)malloc((numElements+1)*sizeof(uint32_t));
      uint32_t* sinksData = (uint32_t*)malloc(numNeighbors*sizeof(uint32_t));

      int currNbrsStart = 0;
      for (auto& p : pathNeighbors) {
        unsigned elem = p.first;
        coordsData[elem] = currNbrsStart;

        unsigned pNeighborSize = p.second.size();
        if (pNeighborSize > 0) {
          vector<unsigned> pNeighbors(p.second.begin(), p.second.end());
          if (sorted) {
            sort(pNeighbors.begin(), pNeighbors.end());
          }

          memcpy(&sinksData[currNbrsStart], pNeighbors.data(),
                 pNeighbors.size() * sizeof(uint32_t));

          currNbrsStart += pNeighborSize;
        }
      }
      coordsData[numElements] = currNbrsStart;
      return new SegmentedPathIndex(numElements, coordsData, sinksData);;
    }

    void visit(const Link *link) {
      switch (link->getType()) {
        case Link::ev: {
          const simit::Set& edgeSet = *builder->getBinding(link->getEdgeSet());

          const int cardinality = edgeSet.getCardinality();
          simit_iassert(cardinality > 0)
              << "not an edge set" << edgeSet.getName();

          const simit::Set& vertexSet =
              *builder->getBinding(link->getVertexSet());

          int nnzPerRow = 0;
          for (size_t i=0; i<(size_t)edgeSet.getCardinality(); ++i) {
            nnzPerRow += (&vertexSet == edgeSet.getEndpointSet(i));
          }

          // TODO: Replace rest of this case with this when we want to support
          //       a mix of segmented and set endpoint indices.
          // pi = new SetEndpointPathIndex(edgeSet);

          size_t n   = edgeSet.getSize();
          size_t nnz = edgeSet.getSize() * nnzPerRow;

          uint32_t* ptr = (uint32_t*)malloc((n+1)*sizeof(uint32_t));
          uint32_t* idx = (uint32_t*)malloc(nnz*sizeof(uint32_t));

          for (size_t i=0; i<=n; ++i) {
            ptr[i] = i*nnzPerRow;
          }

          for (auto e : edgeSet) {
            for (int i=0, j=0; i<cardinality; ++i) {
              if (&vertexSet == edgeSet.getEndpointSet(i)) {
                int ep = edgeSet.getEndpoint(e,i).getIdent();
                idx[e.getIdent()*nnzPerRow + (j++)] = ep;
              }
            }
          }

          pi = new SegmentedPathIndex(n, ptr, idx);;
          break;
        }
        case Link::ve: {
          const simit::Set& edgeSet = *builder->getBinding(link->getEdgeSet());
          simit_iassert(edgeSet.getCardinality() > 0)
              << "not an edge set" << edgeSet.getName();
          
          // add each edge to the neighbor vectors of its endpoints
          map<unsigned, vector<unsigned>> pathNeighbors;

          // create neighbor lists
          const simit::Set& vertexSet =
              *builder->getBinding(link->getVertexSet());
          for (auto &v : vertexSet) {
            pathNeighbors.insert({v.getIdent(), vector<unsigned>()});
          }

          // populate neighbor lists
          for (auto &e : edgeSet) {
            simit_iassert(e.getIdent() >= 0);
            int i = 0;
            for (auto &ep : edgeSet.getEndpoints(e)) {
              simit_iassert(ep.getIdent() >= 0);
              if (&vertexSet == edgeSet.getEndpointSet(i++)) {
                pathNeighbors.at(ep.getIdent()).push_back(e.getIdent());
              }
            }
          }
          pi = pack(pathNeighbors);
          break;
        }
        case Link::vv: {
          const ir::StencilLayout& stencil = link->getStencil();
          const simit::Set& throughSet =
              *builder->getBinding(stencil.getGridSet());
          
          map<unsigned, vector<unsigned>> pathNeighbors;

          // create neighbor lists
          const simit::Set& sourceSet =
              *builder->getBinding(link->getVertexSet(0));
          simit_iassert(sourceSet.getName() ==
                  builder->getBinding(link->getVertexSet(1))->getName());
          for (auto &v : sourceSet) {
            pathNeighbors.insert({v.getIdent(), vector<unsigned>()});
            for (auto &kv : stencil.getLayoutReversed()) {
              const vector<int> &offsets = kv.second;
              vector<int> base = throughSet.getGridPointCoords(v);
              simit_iassert(offsets.size() == base.size());
              for (unsigned i = 0; i < base.size(); ++i) {
                base[i] += offsets[i] + throughSet.getDimensions()[i];
                base[i] = base[i] % throughSet.getDimensions()[i];
              }
              pathNeighbors.at(v.getIdent()).push_back(
                  throughSet.getGridPoint(base).getIdent());
            }
          }
          pi = pack(pathNeighbors, false);
          break;
        }
        default: simit_unreachable;
      }
    }

    static VarToLocationsMap
    getVarToLocationsMap(const vector<PathExpression>& pexprs) {
      VarToLocationsMap varToLocationsMap;
      for (const PathExpression& pexpr : pexprs) {
        for (unsigned ep=0; ep < pexpr.getNumPathEndpoints(); ++ep) {
          Location loc;
          loc.pathExpr = pexpr;
          loc.endpoint = ep;
          varToLocationsMap[pexpr.getPathEndpoint(ep)].push_back(loc);
        }
      }
      return varToLocationsMap;
    }

    PathIndex buildIndex(const PathExpression &pathExpr,
                         const Var &source, const Var &sink) {
      VarToLocationsMap locs = getVarToLocationsMap({pathExpr});
      simit_iassert(util::contains(locs, source))
          << "source variable is not in the path expression";
      simit_iassert(util::contains(locs, sink))
          << "sink variable is not in the path expression";
      return builder->buildSegmented(locs.at(source)[0].pathExpr,
                                     locs.at(source)[0].endpoint);
    }

    tuple<PathIndex,PathIndex> buildIndices(const PathExpression &lhs,
                                            const PathExpression &rhs,
                                            const Var &source,
                                            const Var &quantified,
                                            const Var &sink) {
      VarToLocationsMap varToLocations = getVarToLocationsMap({lhs,rhs});
      simit_iassert(varToLocations.find(quantified) != varToLocations.end())
          << "could not find quantified variable locations";
      simit_iassert(varToLocations[quantified].size() == 2)
          << "quantified binary expr only uses quantified variable once";

      Location sourceLoc = varToLocations[source][0];
      PathIndex sourceToQuantified =
          builder->buildSegmented(sourceLoc.pathExpr, sourceLoc.endpoint);
      PathIndex sourceToQuantified2, quantifiedToSink2;

      Location sinkLoc = varToLocations[sink][0];
      unsigned quantifiedLoc = ((sinkLoc.endpoint) == 0) ? 1 : 0;
      PathIndex quantifiedToSink =
          builder->buildSegmented(sinkLoc.pathExpr, quantifiedLoc);

      return make_pair(sourceToQuantified, quantifiedToSink);
    }

    void visit(const And *f) {
      auto &freeVars = f->getFreeVars();
      simit_iassert(freeVars.size() == 2)
          << "For now, we only support matrix path expressions";

      PathExpression lhs = f->getLhs();
      PathExpression rhs = f->getRhs();

      map<unsigned, set<unsigned>> pathNeighbors;
      if (!f->isQuantified()) {
        // Build indices from first to second free variable through lhs and rhs
        PathIndex lhsIndex = buildIndex(lhs, freeVars[0], freeVars[1]);
        PathIndex rhsIndex = buildIndex(rhs, freeVars[0], freeVars[1]);

        // Build a path index that is the intersection of lhsIndex and rhsIndex.
        // OPT: If path indices supported efficient lookups we could instead:
        //      for each (elem,nbr) pair in lhs, if it exist in rhs then emit.
        map<unsigned, set<unsigned>> lhsPathNeighbors;
        for (unsigned elem : lhsIndex) {
          lhsPathNeighbors.insert({elem, set<unsigned>()});
          for (unsigned nbr : lhsIndex.neighbors(elem)) {
            lhsPathNeighbors.at(elem).insert(nbr);
          }
        }
        for (unsigned elem : rhsIndex) {
          pathNeighbors.insert({elem, set<unsigned>()});
          for (unsigned nbr : rhsIndex.neighbors(elem)) {
            auto &elemNbrs = lhsPathNeighbors.at(elem);
            if (elemNbrs.find(nbr) != elemNbrs.end()) {
              pathNeighbors.at(elem).insert(nbr);
            }
          }
        }
      }
      else {
        simit_iassert(f->getQuantifiedVars().size() == 1)
            << "For now, we only support one quantified variable";

        QuantifiedVar qvar = f->getQuantifiedVars()[0];

        // The expression combines two binary path expressions with one
        // quantified variable. Thus, each operand must link one of the two free
        // variables to the quantified variable.

        // Build indices from the first free variable to the quantified var,
        // and from the quantified var to the second free variable
        PathIndex sourceToQuantified;
        PathIndex quantifiedToSink;

        tie(sourceToQuantified, quantifiedToSink) =
            buildIndices(lhs, rhs, freeVars[0], qvar.getVar(), freeVars[1]);

        // Build a path index from the first free variable to the second free
        // variable, through the quantified variable.
        for (unsigned source : sourceToQuantified) {
          pathNeighbors.insert({source, set<unsigned>()});
          for (unsigned q : sourceToQuantified.neighbors(source)) {
            for (unsigned sink : quantifiedToSink.neighbors(q)) {
              pathNeighbors.at(source).insert(sink);
            }
          }
        }
      }
      // Convert path neighbors to vector values
      map<unsigned, vector<unsigned>> pathNeighborsVec;
      for (auto &kv : pathNeighbors) {
        pathNeighborsVec[kv.first] =
            vector<unsigned>(kv.second.begin(), kv.second.end());
      }
      pi = pack(pathNeighborsVec);
    }

    void visit(const Or *f) {
      auto &freeVars = f->getFreeVars();
      simit_iassert(freeVars.size() == 2)
          << "For now, we only support matrix path expressions";

      PathExpression lhs = f->getLhs();
      PathExpression rhs = f->getRhs();

      map<unsigned, set<unsigned>> pathNeighbors;
      if (!f->isQuantified()) {
        // Build indices from first to second free variable through lhs and rhs
        PathIndex lhsIndex = buildIndex(lhs, freeVars[0], freeVars[1]);
        PathIndex rhsIndex = buildIndex(rhs, freeVars[0], freeVars[1]);

        // Build a path index that is the union of lhsIndex and rhsIndex
        for (unsigned elem : lhsIndex) {
          pathNeighbors.insert({elem, set<unsigned>()});
          for (unsigned nbr : lhsIndex.neighbors(elem)) {
            pathNeighbors.at(elem).insert(nbr);
          }
        }
        for (unsigned elem : rhsIndex) {
          simit_iassert(pathNeighbors.find(elem) != pathNeighbors.end());
          for (unsigned nbr : rhsIndex.neighbors(elem)) {
            pathNeighbors.at(elem).insert(nbr);
          }
        }
      }
      else {
        simit_iassert(f->getQuantifiedVars().size() == 1)
            << "For now, we only support one quantified variable";

        QuantifiedVar qvar = f->getQuantifiedVars()[0];

        // The expression combines two binary path expressions with one
        // quantified variable. Thus, each operand must link one of the two free
        // variables to the quantified variable.

        // Build indices from the first free variable to the quantified var,
        // and from the quantified var to the second free variable
        PathIndex sourceToQuantified;
        PathIndex quantifiedToSink;

        // OPT: The index building algorithm is agnostic to the direction these
        //      indices are built in. We should take advantage by:
        //      - checking whether one direction is already available/memoized
        //      - checking whether one direction is an ev link (which is fast)
        tie(sourceToQuantified, quantifiedToSink) =
            buildIndices(lhs, rhs, freeVars[0], qvar.getVar(), freeVars[1]);

        // Build a path index that from the first free variable to the
        // quantified variable. Every free variable that can reach any
        // quantified variable gets links to every element of the second
        // variable. Vice versa for the second variable, but jump from the
        // quantified var.
        auto sinkSet = builder->getBinding(f->getSet(freeVars[1]));

        for (unsigned source : sourceToQuantified) {
          pathNeighbors.insert({source, set<unsigned>()});
          if (sourceToQuantified.numNeighbors(source) > 0) {
            for (auto &sinkElem : *sinkSet) {
              unsigned sink = sinkElem.getIdent();
              pathNeighbors.at(source).insert(sink);
            }
          }
        }

        for (unsigned quantified : quantifiedToSink) {
          for (unsigned sink: quantifiedToSink.neighbors(quantified)) {
            for (unsigned source : sourceToQuantified) {
              pathNeighbors.at(source).insert(sink);
            }
          }
        }
      }
      // Convert path neighbors to vector values
      map<unsigned, vector<unsigned>> pathNeighborsVec;
      for (auto &kv : pathNeighbors) {
        pathNeighborsVec[kv.first] =
            vector<unsigned>(kv.second.begin(), kv.second.end());
      }
      pi = pack(pathNeighborsVec);
    }

    PathIndex pi;  // Path index returned from cases
    PathIndexBuilder *builder;
  };

  // TODO: Possible optimization is to detect symmetric path expressions, and
  //       return the same path index when they are evaluated in both directions

  // Check if we have memoized the path index for this path expression, starting
  // at this sourceEndpoint, bound to these sets.
  if (util::contains(pathIndices, {pe,sourceEndpoint})) {
    return pathIndices.at({pe,sourceEndpoint});
  }

  PathIndex pi = PathNeighborVisitor(this).build(pe);
  pathIndices.insert({{pe,sourceEndpoint}, pi});
  return pi;
}

void PathIndexBuilder::bind(std::string name, const simit::Set* set) {
  bindings.insert({name,set});
}

const simit::Set* PathIndexBuilder::getBinding(pe::Set pset) const {
  simit_iassert(pset.defined());
  return bindings.at(pset.getName());
}

const simit::Set* PathIndexBuilder::getBinding(ir::Var var) const {
  simit_iassert(var.defined());
  return bindings.at(var.getName());
}

}}

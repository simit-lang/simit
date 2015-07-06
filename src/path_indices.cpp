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

unsigned SetEndpointPathIndex::numNeighbors(const ElementRef &elem) const {
  return edgeSet.getCardinality();
}

unsigned SetEndpointPathIndex::numNeighbors() const {
  return numElements() * edgeSet.getCardinality();
}

class SetEndpointElementIterator : public PathIndexImpl::ElementIterator::Base {
public:
  SetEndpointElementIterator(const Set::ElementIterator &setElemIterator)
      : setElemIterator(setElemIterator) {}

  void operator++() {++setElemIterator;}
  ElementRef& operator*() {return *setElemIterator;}
  Base* clone() const {return new SetEndpointElementIterator(*this);}

protected:
  bool equal(const Base& o) const {
    const SetEndpointElementIterator *other =
        static_cast<const SetEndpointElementIterator*>(&o);
    return setElemIterator == other->setElemIterator;
  }

private:
  Set::ElementIterator setElemIterator;
};

SetEndpointPathIndex::ElementIterator SetEndpointPathIndex::begin() const {
  return new SetEndpointElementIterator(edgeSet.begin());
}

SetEndpointPathIndex::ElementIterator SetEndpointPathIndex::end() const {
  return new SetEndpointElementIterator(edgeSet.end());
}

SetEndpointPathIndex::Neighbors
SetEndpointPathIndex::neighbors(const ElementRef &elem) const {
  class SetEndpointNeighbors : public PathIndexImpl::Neighbors::Base {
    class Iterator : public PathIndexImpl::Neighbors::Iterator::Base {
    public:
      Iterator(const Set::Endpoints::Iterator &epit) : epit(epit) {}

      void operator++() {++epit;}
      ElementRef& operator*() {return *epit;}
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

  return new SetEndpointNeighbors(edgeSet.getEndpoints(elem));
}

void SetEndpointPathIndex::print(std::ostream &os) const {
  os << "SetEndpointPathIndex";
  for (auto &e : *this) {
    os << "\n" << e << ": ";
    for (auto &ep : neighbors(e)) {
      os << ep << " ";
    }
  }
}

// class SegmentedPathIndex
SegmentedPathIndex::SegmentedPathIndex() {
  nbrsStart.push_back(0);
}

unsigned SegmentedPathIndex::numElements() const {
  iassert(nbrsStart.size() > 0);
  return nbrsStart.size() - 1;
}

unsigned SegmentedPathIndex::numNeighbors(const ElementRef &elem) const {
  iassert(elem.getIdent() >= 0 && nbrsStart.size() > (unsigned)elem.getIdent());
  return nbrsStart[elem.getIdent()];
}

unsigned SegmentedPathIndex::numNeighbors() const {
}

SegmentedPathIndex::ElementIterator SegmentedPathIndex::begin() const {
}

SegmentedPathIndex::ElementIterator SegmentedPathIndex::end() const {
}

SegmentedPathIndex::Neighbors
SegmentedPathIndex::neighbors(const ElementRef &elem) const {
}

void SegmentedPathIndex::print(std::ostream &os) const {
  os << "SegmentedPathIndex";
//  for (auto &<#item#> : <#collection#>) {<#body#>}
}


// class PathIndexBuilder
PathIndex PathIndexBuilder::buildCSR(const PathExpression &pe,
                                     unsigned sourceEndpoint,
                                     std::map<ElementVar,const Set&> bindings) {
  // Check if we have memoized the path index for this path expression, starting
  // at this sourceEndpoint, bound to these sets.
  // TODO

  // If it is an EV path expression we return an EndpointPathIndex, that wraps
  // the Edge set.
  if (dynamic_cast<EV*>(pe.ptr) != nullptr) {
    EV *ev = dynamic_cast<EV*>(pe.ptr);
    const Set &edgeSet = bindings[ev->getPathEndpoint(0)];
    return new SetEndpointPathIndex(edgeSet);
  }

  SegmentedPathIndex *pathIndex = new SegmentedPathIndex();

  // Interpret the path expression, starting at sourceEndpoint, over the graph.

  // Given an element, the PathNeighborVisitor finds its neighbors through a
  // path expression.
  class PathNeighborVisitor : public PathExpressionVisitor {
  public:
//    PathNeighborVisitor(const PathIndexBuilder &builder) {
//    }
  private:
    void visit(const EV *ev) {
      std::cout << "Visiting EV" << std::endl;
    };
  };

  PathNeighborVisitor visitor;
  pe.accept(&visitor);

  return pathIndex;
}

}}

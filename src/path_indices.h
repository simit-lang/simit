#ifndef SIMIT_PATH_INDICES_H
#define SIMIT_PATH_INDICES_H

#include <ostream>
#include <map>
#include <memory>
#include <typeinfo>

#include "printable.h"
#include "graph.h"
#include "path_expressions.h"

namespace simit {
namespace pe {
class PathExpression;
class PathIndexBuilder;
class PathIndexImpl;

class PathIndexImpl : public interfaces::Printable {
public:
  class ElementIterator {
  public:
    ElementIterator(unsigned currElem) : currElem(currElem) {}
    ElementIterator(const ElementIterator& it) : currElem(it.currElem) {}
    ElementIterator& operator++() {++currElem; return *this;}

    friend bool operator==(const ElementIterator& lhs,
                           const ElementIterator& rhs) {
      return lhs.currElem == rhs.currElem;
    }

    friend bool operator!=(const ElementIterator& lhs,
                           const ElementIterator& rhs) {
      return lhs.currElem != rhs.currElem;
    }

    const unsigned& operator*() const {return currElem;}

  private:
    unsigned currElem;
  };

  class Neighbors {
  public:
    class Iterator {
    public:
      class Base {
      public:
        Base() {}
        virtual ~Base() {}
        virtual void operator++() = 0;
        virtual unsigned operator*() const = 0;
        virtual Base* clone() const = 0;
        friend bool operator==(const Base &l, const Base &r) {
          return typeid(l) == typeid(r) && l.eq(r);
        }
      protected:
        virtual bool eq(const Base &o) const = 0;
      };

    Iterator() : impl(nullptr) {}
    Iterator(Base *impl) : impl(impl) {}
    Iterator(const Iterator& o) : impl(o.impl->clone()) {}
    ~Iterator() {delete impl;}
    Iterator& operator=(const Iterator& o) {
      if (impl != o.impl) { delete impl; impl = o.impl->clone(); }
      return *this;
    }

    Iterator& operator++() {++(*impl); return *this;}
    unsigned operator*() const {return *(*impl);}
    bool operator==(const Iterator& o) const {
      return (impl == o.impl) || (*impl == *o.impl);
    }
    bool operator!=(const Iterator& o) const {return !(*this == o);}
      
    private:
      Base *impl;
    };

    class Base {
    public:
      virtual Iterator begin() const = 0;
      virtual Iterator end() const = 0;
    };

    Neighbors() : impl(nullptr) {}
    Neighbors(Base *impl) : impl(impl) {}

    Iterator begin() const {return impl->begin();}
    Iterator end() const {return impl->end();}

  private:
    Base *impl;
  };

  virtual ~PathIndexImpl() {}

  virtual unsigned numElements() const = 0;
  virtual unsigned numNeighbors(unsigned elemID) const = 0;
  virtual unsigned numNeighbors() const = 0;

  ElementIterator begin() const {return ElementIterator(0);}
  ElementIterator end() const {return ElementIterator(numElements());}

  virtual Neighbors neighbors(unsigned elemID) const = 0;

private:
  mutable long ref = 0;
  friend inline void aquire(PathIndexImpl *p) {++p->ref;}
  friend inline void release(PathIndexImpl *p) {if (--p->ref==0) delete p;}
};


/// A Path Index enumerates the neighbors of an element through all the paths
/// described by a path expression.
class PathIndex : public util::IntrusivePtr<PathIndexImpl> {
public:
  typedef PathIndexImpl::ElementIterator ElementIterator;
  typedef PathIndexImpl::Neighbors Neighbors;

  PathIndex() : IntrusivePtr(nullptr) {}

  /// The number of elements that this path index maps to their neighbors.
  unsigned numElements() const {return ptr->numElements();}

  /// The number of path neighbors of `elem`.
  unsigned numNeighbors(unsigned elemID) const {
    return ptr->numNeighbors(elemID);
  }

  /// The sum of number of neighbors of each element covered by this path index.
  unsigned numNeighbors() const {return ptr->numNeighbors();}

  /// Iterator that iterates over the elements covered by this path index.
  ElementIterator begin() const {return ptr->begin();}
  ElementIterator end() const {return ptr->end();}

  /// Get the neighbors of `elem` through this path index.
  Neighbors neighbors(unsigned elemID) const {
    return ptr->neighbors(elemID);
  }

  friend std::ostream &operator<<(std::ostream&, const PathIndex&);

private:
  /// PathIndex objects are constructed through a PathIndexBuilder.
  PathIndex(PathIndexImpl *impl) : IntrusivePtr(impl) {}
  friend PathIndexBuilder;
};


/// A SetEndpointPathIndex uses a Set's endpoint list to find path neighbors.
class SetEndpointPathIndex : public PathIndexImpl {
public:
  unsigned numElements() const;
  unsigned numNeighbors(unsigned elemID) const;
  unsigned numNeighbors() const;

  Neighbors neighbors(unsigned elemID) const;

private:
  const Set &edgeSet;

  void print(std::ostream &os) const;

  friend PathIndexBuilder;
  SetEndpointPathIndex(const Set &edgeSet);
};


/// In a SegmentedPathIndex the path neighbors are packed into a segmented
/// vector with no holes. This is equivalent to CSR indices.
class SegmentedPathIndex : public PathIndexImpl {
public:
  ~SegmentedPathIndex() {
    if (nbrsStart != nullptr) delete nbrsStart;
    if (nbrs != nullptr) delete nbrs;
  }

  unsigned numElements() const {return numElems;}

  unsigned numNeighbors(unsigned elemID) const {
    iassert(numElems > elemID);
    return nbrsStart[elemID+1]-nbrsStart[elemID];
  }

  unsigned numNeighbors() const {return nbrsStart[numElems];}

  Neighbors neighbors(unsigned elemID) const;

private:
  /// Segmented vector, where `nbrsStart[i]:nbrsStart[i+1]` is the range of
  /// locations of neighbors of `i` in `nbrs`.
  unsigned numElems;
  unsigned *nbrsStart;
  unsigned *nbrs;

  void print(std::ostream &os) const;

  friend PathIndexBuilder;

  SegmentedPathIndex(unsigned numElements, unsigned *nbrsStart, unsigned *nbrs)
      : numElems(numElements), nbrsStart(nbrsStart), nbrs(nbrs) {}

  SegmentedPathIndex() : numElems(0), nbrsStart(nullptr), nbrs(nullptr) {
    nbrsStart = new unsigned[1];
    nbrsStart[0] = 0;
  }
};


/// A builder that builds path indices by evaluating path expressions on graphs.
/// The builder memoizes previously computed path indices, and uses these to
/// accelerate subsequent path index construction (since path expressions can be
/// recursively constructed from path expressions).
class PathIndexBuilder {
public:
  PathIndexBuilder() {}

  // Build a Segmented path index by evaluating the `pe` over the given graph.
  PathIndex buildSegmented(const PathExpression &pe, unsigned sourceEndpoint);

private:
  std::map<std::pair<PathExpression,unsigned>, PathIndex> pathIndices;
};

}}

#endif

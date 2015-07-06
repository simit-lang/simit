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
    class Base {
    public:
      Base() {}
      virtual ~Base() {}
      virtual void operator++() = 0;
      virtual ElementRef& operator*() = 0;
      virtual Base* clone() const = 0;
      bool operator==(const Base& o) const {
        return typeid(*this) == typeid(o) && equal(o);
      }
    protected:
      virtual bool equal(const Base& o) const = 0;
    };

    ElementIterator() : impl(nullptr) {}
    ElementIterator(Base *impl) : impl(impl) {}
    ElementIterator(const ElementIterator& o) : impl(o.impl->clone()) {}
    ~ElementIterator() { delete impl; }
    ElementIterator& operator=(const ElementIterator& o) {
      if (impl != o.impl) { delete impl; impl = o.impl->clone(); }
      return *this;
    }

    ElementIterator& operator++() {++(*impl); return *this;}
    ElementRef& operator*() const {return *(*impl);}
    bool operator==(const ElementIterator& o) const {
      return (impl == o.impl) || (*impl == *o.impl);
    }
    bool operator!=(const ElementIterator& o) const {return !(*this == o);}

  private:
    Base *impl;
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
        virtual ElementRef& operator*() = 0;
        virtual Base* clone() const = 0;
        bool operator==(const Base& o) const {
          return typeid(*this) == typeid(o) && equal(o);
        }
      protected:
        virtual bool equal(const Base& o) const = 0;
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
    ElementRef& operator*() const {return *(*impl);}
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
  virtual unsigned numNeighbors(const ElementRef &elem) const = 0;
  virtual unsigned numNeighbors() const = 0;

  virtual ElementIterator begin() const = 0;
  virtual ElementIterator end() const = 0;

  virtual Neighbors neighbors(const ElementRef &elem) const = 0;

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

  /// The number of elements that this path index maps to their neighbors.
  unsigned numElements() const {return ptr->numElements();}

  /// The number of path neighbors of `elem`.
  unsigned numNeighbors(const ElementRef &elem) const {
    return ptr->numNeighbors(elem);
  }

  /// The sum of number of neighbors of each element covered by this path index.
  unsigned numNeighbors() const {return ptr->numNeighbors();}

  /// Iterator that iterates over the elements covered by this path index.
  ElementIterator begin() const {return ptr->begin();}
  ElementIterator end() const {return ptr->end();}

  /// Get the neighbors of `elem` through this path index.
  Neighbors neighbors(const ElementRef &elem) const {
    return ptr->neighbors(elem);
  }

  friend std::ostream &operator<<(std::ostream&, const PathIndex&);

private:
  PathIndex(PathIndexImpl *impl) : IntrusivePtr(impl) {}
  friend PathIndexBuilder;
};

/// In a PartitionedPathIndex each element has a fixed number of neighbors.
//class PartitionedPathIndex : public PathIndexImpl {};


/// A SetEndpointPathIndex uses a Set's endpoint list to find path neighbors.
class SetEndpointPathIndex : public PathIndexImpl {
public:
  unsigned numElements() const;
  unsigned numNeighbors(const ElementRef &elem) const;
  unsigned numNeighbors() const;

  ElementIterator begin() const;
  ElementIterator end() const;

  Neighbors neighbors(const ElementRef &elem) const;

private:
  const Set &edgeSet;

  void print(std::ostream &os) const;

  friend PathIndexBuilder;
  SetEndpointPathIndex(const Set &edgeSet);
};


/// A SegmentedPathIndex is one in which the path neighbors are packed into a
/// segmented vector with no holes (i.e. a packed index).
class SegmentedPathIndex : public PathIndexImpl {
public:
  unsigned numElements() const;
  unsigned numNeighbors(const ElementRef &elem) const;
  unsigned numNeighbors() const;

  ElementIterator begin() const;
  ElementIterator end() const;

  Neighbors neighbors(const ElementRef &elem) const;

private:
  /// Segmented vector, where `nbrsStart[i]:nbrsStart[i+1]` is the range of
  /// locations of neighbors of `i` in `nbrs`.
  std::vector<unsigned> nbrsStart;
  std::vector<unsigned> nbrs;

  void print(std::ostream &os) const;

  friend PathIndexBuilder;
  SegmentedPathIndex();
};


/// A builder that builds path indices by evaluating path expressions on graphs.
/// The builder memoizes previously computed path indices, and uses these to
/// accelerate subsequent path index construction (since path expressions can be
/// recursively constructed from path expressions).
class PathIndexBuilder {
public:
  PathIndexBuilder() {}

  // Build a CSR path index by evaluating the `pe` over the given graph.
  PathIndex buildCSR(const PathExpression &pe, unsigned sourceEndpoint,
                     std::map<ElementVar,const Set&> bindings);

private:
  std::map<PathExpression,PathIndex> pathIndices;
};

}}

#endif

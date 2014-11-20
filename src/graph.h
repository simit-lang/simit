#ifndef SIMIT_GRAPH_H
#define SIMIT_GRAPH_H

#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <ostream>

#include "tensor_components.h"
#include "variadic.h"
#include "error.h"

namespace simit {

// Forward declarations
class SetBase;
template <typename T, int... dimensions> class FieldRef;
template <typename T, int... dimensions> class TensorRef;

namespace {
class FieldRefBase;
}
namespace {
namespace hidden {
template <int cardinality> class EndpointIteratorBase;
}
}

namespace internal {
class VertexToEdgeEndpointIndex;
class VertexToEdgeIndex;
}

class Function;

/// A Simit element reference.  All Simit elements live in Simit sets and an
/// ElementRef provides a reference to an element.
class ElementRef {
public:
  inline ElementRef() : ident(-1) {}

  bool defined() const {return ident != -1;}

  friend inline bool operator==(const ElementRef& e1, const ElementRef& e2) {
    return e1.ident == e2.ident;
  }

  friend inline bool operator!=(const ElementRef& e1, const ElementRef& e2) {
    return e1.ident != e2.ident;
  }

  friend inline bool operator<(const ElementRef& e1, const ElementRef& e2) {
    return e1.ident < e2.ident;
  }

  friend inline bool operator>(const ElementRef& e1, const ElementRef& e2) {
    return e1.ident > e2.ident;
  }

  friend inline bool operator<=(const ElementRef& e1, const ElementRef& e2) {
    return e1.ident <= e2.ident;
  }

  friend inline bool operator>=(const ElementRef& e1, const ElementRef& e2) {
    return e1.ident >= e2.ident;
  }

private:
  explicit inline ElementRef(int ident) : ident(ident) {}
  int ident;

  friend SetBase;
  template <int cardinality> friend class Set;
  template <int cardinality> friend class hidden::EndpointIteratorBase;
  friend FieldRefBase;
  friend class internal::VertexToEdgeEndpointIndex;
  friend class internal::VertexToEdgeIndex;
};


// Base class for Sets
// Sets are used to represent collections within C++,
// and can be passed as bound inputs to Simit programs.
class SetBase {
public:
  SetBase() : elements(0), capacity(capacityIncrement), endpoints(nullptr) { }
  
  ~SetBase() {
    for (auto f: fields)
      delete f;
  }
  
  /// Return the number of elements in the Set
  int getSize() const { return elements; }

  /// Return the number of endpoints of the elements in the set.  Non-edge sets
  /// have cardinality 0.
  virtual int getCardinality() const { return 0; }

  /// Add a tensor field to the set.  Use the template parameters to specify the
  /// component type and dimension sizes of the tensors.  For example, define a
  /// field of 2x3 matrices containing doubles as follows:
  /// Field<double,2,3> matrix = addField<double,2,3>("mat");
  template <typename T, int... dimensions>
  FieldRef<T, dimensions...> addField(const std::string &name) {
    FieldData::TensorType *type =
        new FieldData::TensorType(typeOf<T>(), {dimensions...});
    FieldData *fieldData = new FieldData(name, type, this);
    fieldData->data = calloc(capacity, fieldData->sizeOfType);
    fields.push_back(fieldData);
    fieldNames[name] = fields.size()-1;
    return FieldRef<T, dimensions...>(fieldData);
  }
  
  /// Get a Field corresponding to the string fieldName
  template <typename T, int... dimensions>
  FieldRef<T, dimensions...> getField(std::string fieldName) {
    // need to check if the field actually exists because maps just add an entry
    // if none exists
    assert(fieldNames.find(fieldName) != fieldNames.end() &&
      "Invalid field name in getField()");
    FieldData *fieldData = fields[fieldNames[fieldName]];
    assert(typeOf<T>() == fieldData->type->getComponentType() &&
           "Incorrect field type.");
    return FieldRef<T, dimensions...>(fieldData);
  }

  /// Add a new element, returning its handle
  ElementRef addElement() {
    if (elements > capacity-1)
      increaseCapacity();
    return ElementRef(elements++);
  }

  /// Remove an element from the Set
  void remove(ElementRef element) {
    for (auto f : fields){
      iassert(isValidComponentType(f->type->getComponentType()));
      switch (f->type->getComponentType()) {
        case ComponentType::FLOAT: {
          double* data = (double*)f->data;
          data[element.ident] = data[elements-1];
          break;
        }
        case ComponentType::INT: {
          int* data = (int*)f->data;
          data[element.ident] = data[elements-1];
          break;
        }
      }
    }
    elements--;
  }

  /// Iterator that iterates over the elements in a Set
  ///
  /// This iterator is an input_iterator, and thus can only be
  /// dereferenced as an rvalue.  Furthermore, it can only return
  /// const references to Elements.
  class ElementIterator {
  public:
    // some typedefs to make interop with std easier
    typedef std::input_iterator_tag iterator_category;
    typedef ElementRef value_type;
    typedef ptrdiff_t difference_type;
    typedef ElementRef& reference;
    typedef ElementRef* pointer;
    
    ElementIterator(const SetBase* set, int idx=0) : curElem(idx), set(set) { }
    ElementIterator(const ElementIterator& other) : curElem(other.curElem),
    set(other.set) { }
    
    friend inline bool operator<(const SetBase::ElementIterator& e1,
                                 const SetBase::ElementIterator& e2);
    
    reference operator*() {
      return curElem;
    }
    
    pointer operator->()  {
      return &curElem;
    }
    
    ElementIterator& operator++() {
      curElem.ident++;
      return *this;
    }
    
    ElementIterator operator++(int) {
      curElem.ident++;
      return *this;
    }
    
    bool operator!=(const ElementIterator& other) {
      return !(set==other.set) || !(curElem.ident == other.curElem.ident);
    }
    
    bool operator==(const ElementIterator& other) {
      return (set==other.set) && (curElem.ident == other.curElem.ident);
    }

  private:
    ElementRef curElem; // current element index
    const SetBase* set; // set we're iterating over

    bool lessThan(const SetBase::ElementIterator &other) const {
      return this->curElem.ident < other.curElem.ident;
    }

    friend bool operator<(const SetBase::ElementIterator& e1,
                          const SetBase::ElementIterator& e2);
  };
  
  /// Create an ElementIterator for this Set, set to the first element
  ElementIterator begin() const { return ElementIterator(this, 0); }
  
  /// Create an ElementIterator for terminating iteration over this Set
  ElementIterator end() const { return ElementIterator(this, getSize()); }

  friend std::ostream &operator<<(std::ostream &os, const SetBase &set) {
    os << "{";
    auto it = set.begin();
    auto end = set.end();
    if (it != end) {
      os << it->ident;
      if (set.getCardinality() > 0) {
        os << ":(";
        os << set.endpoints[0];
        for (int i=1; i<set.getCardinality(); ++i) {
          os << "," << set.endpoints[i];
        }
        os << ")";
      }
      ++it;
    }
    while (it != end) {
      os << ", " << it->ident;
      if (set.getCardinality() > 0) {
        os << ":(";
        os << set.endpoints[it->ident + 0];
        for (int i=1; i<set.getCardinality(); ++i) {
          os << "," << set.endpoints[it->ident + i];
        }
        os << ")";
      }
      ++it;
    }
    return os << "}";
  }

protected:
  int elements;                      // number of elements in the set
  int capacity;                   // current capacity of the set
  int* endpoints;                           // container for edges
  static const int capacityIncrement = 1024; // increment for capacity increases

private:

  // A field on the members of the Set.
  //
  // Invariant: elements < capacity
  struct FieldData {
    class TensorType {
    public:
      TensorType(ComponentType componentType, std::initializer_list<int> dims)
          : componentType(componentType), dimensions(dims), size(1) {
        for (auto dim : dimensions) {
          size *= dim;
        }
      }
      ComponentType getComponentType() const { return componentType; }
      size_t getOrder() const { return dimensions.size(); }
      size_t getDimension(size_t i) const {
        iassert(i<getOrder());
        return dimensions[i];
      }
      size_t getSize() const { return size; }
    private:
      ComponentType componentType;
      std::vector<int> dimensions;
      size_t size;
    };

    std::string name;
    const TensorType *type;
    size_t sizeOfType;
    SetBase *set;  // Set this field is a member of. Used for printing, etc.
    
    FieldData(const std::string &name, const TensorType *type, SetBase *set)
        : name(name), type(type), set(set), data(nullptr) {
      sizeOfType = componentSize(type->getComponentType()) * type->getSize();
    }

    ~FieldData() {
      if (data != nullptr) free(data);
      delete type;
    }
    
    void* data;         // buffer for the data

    /// Field references so that we can update their data pointers if we realloc
    /// field data.
    //  Note: his is a bit too complex, but avoids two loads on field get/set.
    std::set<FieldRefBase*> fieldReferences;
    
    // disable copy constructors
  private:
    FieldData(const FieldData& f);
    FieldData& operator=(const FieldData& f);
  };

  std::vector<FieldData*> fields;          // fields of the elements in the set
  std::map<std::string, int> fieldNames; // name to field lookups
  
  // disable copy constructors
  SetBase(const SetBase& s);
  SetBase& operator=(const SetBase& s);

  // increase capacity of all fields
  void increaseCapacity();

  friend FieldRefBase;
  friend Function;
};

inline bool operator<(const SetBase::ElementIterator& e1,
                      const SetBase::ElementIterator& e2) {
  iassert(e1.set == e2.set);
  return e1.lessThan(e2);
}

  
template <int cardinality=0>
class Set : public SetBase {
public:
  template <typename ...T>
  Set(const T& ... sets) : SetBase() {
    static_assert(sizeof...(sets) == cardinality,
                  "Wrong number of endpoint sets");

    // TODO: this may not be the most efficient, but does it matter?
    endpointSets = epsMaker(endpointSets, sets...);
    
    endpoints = (int*) calloc(sizeof(int), capacity*cardinality);
  }
  
  Set() : SetBase() {
    iassert(cardinality == 0)
        << "Sets with cardinality>0 must provide sets for endpoints";
  }
  
  ~Set() {
    free(endpoints);
  }

  int getCardinality() const { return cardinality; }
  
  /// Add an edge.
  /// The endpoints refer to the respective Sets they come from.
  template <typename ...T>
  ElementRef addElement(T ... endpoints) {
    
    iassert(sizeof...(endpoints) == cardinality) <<"Wrong number of endpoints.";
    
    if (elements > capacity-1)
      increaseEdgeCapacity();
    
    addHelper(0, endpoints...);
    return SetBase::addElement();
  }
  
  ElementRef addElement() {
    iassert(cardinality == 0) << "Must provide endpoints for cardinality > 0";
    return SetBase::addElement();
  }
  
  /// Get an endpoint of an edge.
  ElementRef getEndpoint(ElementRef edge, int endpointNum) {
    // TODO: may want to use a pool of ElementRefs instead of creating
    // new ones each time
    return ElementRef(endpoints[edge.ident*cardinality+endpointNum]);
  }
  
  /// Iterator that iterates over the endpoints of an edge
  class EndpointIterator : public hidden::EndpointIteratorBase<cardinality> {
  public:
    EndpointIterator(Set<cardinality>* set, ElementRef elem, int endpointN=0)
        : hidden::EndpointIteratorBase<cardinality>(set, elem, endpointN) { }

    EndpointIterator(const EndpointIterator& other) :
    hidden::EndpointIteratorBase<cardinality>(other) { }
  };
  
  /// Start iterator for endpoints of an edge
  EndpointIterator endpoints_begin(ElementRef edge) {
    return EndpointIterator(this, edge, 0);
  }
  
  /// End iterator for endpoints of an edge
  EndpointIterator endpoints_end(ElementRef edge) {
    return EndpointIterator(this, edge, cardinality);
  }

private:
  std::vector<const SetBase*> endpointSets; // sets that the endpoints belong to
  
  void increaseEdgeCapacity() {
    size_t newSize = (capacity+capacityIncrement)*cardinality*sizeof(int);
    endpoints = (int*)realloc(endpoints, newSize);
  }
  
  template <int c> friend class hidden::EndpointIteratorBase;
  friend class internal::VertexToEdgeEndpointIndex;
  friend class internal::VertexToEdgeIndex;
  
  // helper for constructing
  template <typename F, typename ...T>
  std::vector<const SetBase*> epsMaker(std::vector<const SetBase*> sofar,
                                      const F& f, const T& ... sets) {
    sofar.push_back(&f);
    return epsMaker(sofar, sets...);
  }
  template <typename F>
  std::vector<const SetBase*> epsMaker(std::vector<const SetBase*> sofar,
                                      const F& f) {
    sofar.push_back(&f);
    return sofar;
  }
  
  // helper for adding edges
  template <typename F, typename ...T>
  void addHelper(int which, F f, T ... eps) {
    uassert(endpointSets[which]->getSize() > f.ident)
        << "Invalid member of set in addEdge";
    endpoints[elements*cardinality+which] = f.ident;
    addHelper(which+1, eps...);
  }
  template <typename F>
  void addHelper(int which, F f) {
    uassert(endpointSets[which]->getSize() > f.ident)
        << "Invalid member of set in addEdge";
    endpoints[elements*cardinality+which] = f.ident;
  }
};

namespace internal {

/// A class for an index that maps from points to edges that contain that point
/// differentiating between different endpoints
class VertexToEdgeEndpointIndex {
 public:
  template <int cardinality=0>
  VertexToEdgeEndpointIndex(Set<cardinality>& edgeSet) {
    totalEdges = edgeSet.getSize();
    for (auto es : edgeSet.endpointSets) {
      endpointSets.push_back((SetBase*)es);
      
      // allocate array to contain how many edges each element is part of
      // calloc sets them all to zero
      numEdgesForVertex.push_back((int*)calloc(sizeof(int),es->getSize()));
    
      // allocate array to contain which edges each element is part of
      whichEdgesForVertex.push_back((int**)calloc(sizeof(int*),es->getSize()));
      for (int i=0; i<es->getSize(); i++)
        whichEdgesForVertex[endpointSets.size()-1][i] = (int*)malloc(sizeof(int)*
          totalEdges);
    }
    
    for (auto e : edgeSet) {
      for (int epi=0; epi<(int)(endpointSets.size()); epi++) {
        auto ep = edgeSet.getEndpoint(e, epi);
          whichEdgesForVertex[epi][ep.ident][numEdgesForVertex[epi][ep.ident]++] =
          e.ident;
      }
    }
  }
  
  int getNumEdgesForElement(ElementRef vertex, int whichEndpoint) {
    return numEdgesForVertex[whichEndpoint][vertex.ident];
  }
  
  int* getWhichEdgesForElement(ElementRef vertex, int whichEndpoint) {
    return whichEdgesForVertex[whichEndpoint][vertex.ident];
  }
  
  std::vector<int*> getNumEdges() { return numEdgesForVertex; }
  
  std::vector<int**> getWhichEdges() { return whichEdgesForVertex; }
  
  int getTotalEdges() { return totalEdges; }
  
  ~VertexToEdgeEndpointIndex() {
    for (auto ne : numEdgesForVertex)
      free(ne);
    for (int w=0; w<(int)(endpointSets.size()); w++) {
      for (int i=0; i<endpointSets[w]->getSize(); i++)
        free(whichEdgesForVertex[w][i]);
    free(whichEdgesForVertex[w]);
    }
  }
 private:
  
  std::vector<SetBase*> endpointSets;       // the endpoint sets
  std::vector<int*> numEdgesForVertex;      // number of edges the v belongs to
  std::vector<int**> whichEdgesForVertex;   // which edges v belongs to
  int totalEdges;
  

};

/// A class for an index that maps from points to edges that contain that point
/// with no differentiation by endpoint
class VertexToEdgeIndex {
 public:
  template <int cardinality=0>
  VertexToEdgeIndex(Set<cardinality>& edgeSet) {
    totalEdges = edgeSet.getSize();
    for (auto es : edgeSet.endpointSets) {
      auto endpointSet = (SetBase*)es;
      
      endpointSets.push_back(endpointSet);
      
      // if we already have this set in the container, don't insert again.
      // this case occurs if the edgeset is homogenous, for example.
      if (numEdgesForVertex.count(endpointSet) > 0)
        continue;

      
      // allocate array to contain how many edges each element is part of
      // calloc sets them all to zero
      numEdgesForVertex[endpointSet] = (int*)calloc(sizeof(int), es->getSize());
    
      // allocate array to contain which edges each element is part of
      whichEdgesForVertex[endpointSet] = (int**)calloc(sizeof(int*),
                                                       es->getSize());
      for (int i=0; i<endpointSet->getSize(); i++)
        whichEdgesForVertex[endpointSet][i] = (int*)malloc(sizeof(int)*
                                                           totalEdges);
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
  
  int getNumEdgesForElement(ElementRef vertex, const SetBase& whichSet) {
    return numEdgesForVertex[&whichSet][vertex.ident];
  }
  
  int* getWhichEdgesForElement(ElementRef vertex, const SetBase& whichSet) {
    return whichEdgesForVertex[&whichSet][vertex.ident];
  }
  
  std::map<const SetBase*,int*> getNumEdges() { return numEdgesForVertex; }
  
  std::map<const SetBase*,int**> getWhichEdges() { return whichEdgesForVertex; }
  
  int getTotalEdges() { return totalEdges; }
  
  ~VertexToEdgeIndex() {
    for (auto ne : numEdgesForVertex) {
      free(ne.second);
    }
    for (auto s : whichEdgesForVertex) {
      for (int i=0; i<s.first->getSize(); i++)
        free(s.second[i]);
      free(s.second);
    }
  }
  
 private:
  std::vector<SetBase*> endpointSets;                 // the endpoint sets
  std::map<const SetBase*,int*> numEdgesForVertex;    // number of edges the v
                                                      // belongs to
  std::map<const SetBase*,int**> whichEdgesForVertex; // which edges v belongs to
  int totalEdges;
};

} // internal

namespace {
namespace hidden {
// Base class for iterator that iterates over the endpoints in an edge
template<int cardinality>
class EndpointIteratorBase {
public:
  // some typedefs to make interop with std easier
  typedef std::input_iterator_tag iterator_category;
  typedef ElementRef value_type;
  typedef ptrdiff_t difference_type;
  typedef ElementRef& reference;
  typedef ElementRef* pointer;
  
  EndpointIteratorBase(Set<cardinality>* set, ElementRef elem, int endpointN=0) :
  curElem(elem), retElem(-1), endpointNum(endpointN), set(set) {
    retElem = set->getEndpoint(curElem, endpointNum);
  }
  EndpointIteratorBase(const EndpointIteratorBase& other) : curElem(other.curElem),
  retElem(other.retElem), endpointNum(other.endpointNum), set(other.set) { }
  
  reference operator*() {
    return retElem;
  }
  
  pointer operator->()  {
    return &retElem;
  }
  
  EndpointIteratorBase& operator++() {
    endpointNum++;
    if (endpointNum > cardinality-1)
      retElem.ident = -1;   // return invalid element
    else
      retElem.ident = set->endpoints[curElem.ident*cardinality+endpointNum];
    return *this;
  }
  
  EndpointIteratorBase operator++(int) {
    endpointNum++;
    if (endpointNum > cardinality-1)
      retElem.ident = -1;   // return invalid element
    else
      retElem.ident = set->endpoints[curElem.ident*cardinality+endpointNum];
    return *this;
  }
  
  bool operator!=(const EndpointIteratorBase& other) {
    return !(set==other.set) || !(curElem.ident == other.curElem.ident) ||
    !(endpointNum==other.endpointNum);
  }
  
  bool operator==(const EndpointIteratorBase& other) {
    return (set==other.set) && (curElem.ident == other.curElem.ident) &&
    (endpointNum==other.endpointNum);
  }
  
  bool lessThan(const EndpointIteratorBase &other) const {
    assert(this->set == other.set &&
           "Comparing EndpointIterators from two different Sets");
    assert(this->curElem.ident == other.curElem.ident &&
           "Comparing EndpointIterators over two different edges");
    return this->endpointNum < other.endpointNum;
  }
  
private:
  ElementRef curElem;     // current element index
  ElementRef retElem;     // element we're returning
  int endpointNum;        // the current endpoint number
  Set<cardinality>* set;           // set we're iterating over
};
  
template <int c>
  inline bool operator<(const EndpointIteratorBase<c>& e1,
                      const EndpointIteratorBase<c>& e2) {
  return e1.lessThan(e2);
}
}  // hidden namespace
}  // unnamed namespace


// Field References

namespace {

/// The base class of field references.
class FieldRefBase {
public:
   /// Rule of five methods for copying and moving the field reference.  Note
   /// that there is quite a bit of machinery to maintain a fieldReferences
   /// set of live field references in Set::FieldData.  This is because the
   /// field data may be reallocated in which case the field reference data
   /// pointers must be updated.
  ~FieldRefBase() {
    this->fieldData->fieldReferences.erase(this);
  }

  FieldRefBase(const FieldRefBase& other) {
    data = other.data;
    fieldData = other.fieldData;
    this->fieldData->fieldReferences.insert(this);
  }

  FieldRefBase(FieldRefBase&& other) {
    std::swap (data, other.data);
    std::swap (fieldData, other.fieldData);
    this->fieldData->fieldReferences.erase(&other);
    this->fieldData->fieldReferences.insert(this);
  }

  FieldRefBase& operator=(const FieldRefBase &other) {
    data = other.data;
    fieldData = other.fieldData;
    this->fieldData->fieldReferences.insert(this);
    return *this;
  }

  FieldRefBase& operator=(FieldRefBase&& other) {
    std::swap(data, other.data);
    std::swap (fieldData, other.fieldData);
    this->fieldData->fieldReferences.erase(&other);
    this->fieldData->fieldReferences.insert(this);
    return *this;
  }

  // Return the field's data.  The data is a contigues sequence containing the
  // tensor of each element in no particular order.  The tensors are currently
  // laid out in row-major order, but this may change in the future.
  inline void *getData() {
    return static_cast<void*>(data);
  }

protected:
  FieldRefBase(void *fieldData)
      : fieldData(static_cast<SetBase::FieldData*>(fieldData)),
        data(this->fieldData->data) {
    this->fieldData->fieldReferences.insert(this);
  }

  template <typename T>
  inline T *getElemDataPtr(ElementRef element, size_t elementFieldSize) const {
    return &static_cast<T*>(data)[element.ident * elementFieldSize];
  }

  SetBase::FieldData *fieldData;

private:
  void *data;

  friend SetBase;
};

template <typename T, int... dimensions>
class FieldRefBaseParameterized : public FieldRefBase {
 public:
  TensorRef<T, dimensions...> get(ElementRef element) {
    return TensorRef<T, dimensions...>(getElemDataPtr(element));
  }

  const TensorRef<T, dimensions...> get(ElementRef element) const {
    return TensorRef<T, dimensions...>(getElemDataPtr(element));
  }

  void set(ElementRef element, std::initializer_list<T> values) {
    size_t tensorSize = TensorRef<T,dimensions...>::getSize();
    assert(values.size() == tensorSize && "Incorrect number of init values");
    T *elemData = this->getElemDataPtr(element);
    size_t i=0;
    for (T val : values) {
      elemData[i++] = val;
    }
  }

 protected:
  inline T *getElemDataPtr(ElementRef element) const {
    size_t elementFieldSize = TensorRef<T,dimensions...>::getSize();
    return FieldRefBase::getElemDataPtr<T>(element, elementFieldSize);
  }

  FieldRefBaseParameterized(void *fieldData) : FieldRefBase(fieldData) {}
};


} // unnamed namespace

template <typename T, int... dimensions>
class FieldRef : public FieldRefBaseParameterized<T,dimensions...> {
 private:
  FieldRef(void *fieldData)
      : FieldRefBaseParameterized<T,dimensions...>(fieldData) {}
  friend class SetBase;

  friend std::ostream &operator<<(std::ostream &os,
                                  const FieldRef<T, dimensions...> &field) {
    os << "[";
    auto it = field.fieldData->set->begin();
    auto end = field.fieldData->set->end();
    if (it != end) {
      os << field.get(*it);
      ++it;
    }

    while (it != end) {
      os << ", " << field.get(*it);
      ++it;
    }

    return os << "]";
  }
};

/// @cond SPECIALIZATION
template <typename T>
class FieldRef<T> : public FieldRefBaseParameterized<T> {
 public:
  void set(ElementRef element, T val) {
    (*this->getElemDataPtr(element)) = val;
  }

 private:
  FieldRef(void *fieldData) : FieldRefBaseParameterized<T>(fieldData) {}
  friend class SetBase;
};
/// @endcond

// Tensor References

template <typename T, int... dimensions>
class TensorRef {
 public:
  static size_t getOrder() {
    return sizeof...(dimensions);
  }

  static size_t getSize() { return util::product<dimensions...>::value; }

  inline TensorRef<T> &operator=(T val) {
    static_assert(sizeof...(dimensions) == 0,
                  "Can only assign scalar values to scalar tensors.");
    data[0] = val;
    return *this;
  }

  inline operator T() const {
    static_assert(sizeof...(dimensions) == 0,
                  "Can only convert scalar tensors to scalar values.");
    return data[0];
  }

  template <typename... Indices>
  inline T &operator()(Indices... index) {
    static_assert(sizeof...(dimensions) > 0,
                  "Access scalars directly, not through operator()");
    static_assert(sizeof...(index) == sizeof...(dimensions),
                  "Incorrect number of indices used to index tensor");
    auto dims = simit::util::seq<dimensions...>();
    return data[simit::util::computeOffset(dims, index...)];
  }

  template <typename... Indices>
  inline const T &operator()(Indices... index) const {
    return const_cast<TensorRef<T,dimensions...>*>(this)->operator()(index...);
  }

 private:
  inline TensorRef(T *data) : data(data) {}
  T *data;

  friend class FieldRefBaseParameterized<T, dimensions...>;
};

template <typename T, int... dims>
std::ostream &operator<<(std::ostream &os, const TensorRef<T, dims...> & t) {
  ierror << "General tensor operator<< not yet supported";
  return os;
}

template <typename T, int size>
std::ostream &operator<<(std::ostream &os, const TensorRef<T, size> &t) {
  os << "[";
  if (0 < size) {
    os << t(0);
  }

  for (int i=1; i<size; ++i) {
    os << ", " << t(i);
  }
  return os << "]";
}

template <typename T, int r, int c>
std::ostream &operator<<(std::ostream &os, const TensorRef<T, r, c> &t) {
  os << "[";
  if (0 < r) {
    if (0 < c) {
      os << t(0,0);
    }
    for (int j=1; j<c; ++j) {
      os << ", " << t(0,j);
    }
  }

  for (int i=0; i<r; ++i) {
    os << "; ";
    if (0 < c) {
      os << t(i,0);
    }
    for (int j=1; j<c; ++j) {
      os << ", " << t(i,j);
    }
  }
  return os << "]";
}

// Graph generators
void createElements(Set<> *elements, unsigned num);

class Box {
public:
  typedef std::pair<ElementRef,ElementRef> Coord;

  Box(unsigned nX, unsigned nY, unsigned nZ, std::vector<ElementRef> refs,
      std::map<Box::Coord, ElementRef> coords2edges)
      : nX(nX), nY(nY), nZ(nZ), refs(refs), coords2edges(coords2edges) {
    iassert(refs.size() == nX*nY*nZ);
  }

  unsigned numX() const {return nX;}
  unsigned numY() const {return nY;}
  unsigned numZ() const {return nZ;}

  ElementRef operator()(unsigned x, unsigned y, unsigned z) {
    return refs[z*nY*nX + y*nX + x];
  }

  ElementRef getEdge(ElementRef p1, ElementRef p2) const {
    Coord coord(p1,p2);
    if (coords2edges.find(coord) == coords2edges.end()) {
      return ElementRef();
    }
    return coords2edges.at(coord);
  }

  std::vector<ElementRef> getEdges() {
    std::vector<ElementRef> edges;
    for (auto &coord2edge : coords2edges) {
      edges.push_back(coord2edge.second);
    }
    return edges;
  }

private:
  unsigned nX, nY, nZ;
  std::vector<ElementRef> refs;
  std::map<Coord, ElementRef> coords2edges;
};

Box createBox(Set<> *elements, Set<2> *edges,
              unsigned numX, unsigned numY, unsigned numZ);


} // namespace simit

#endif

#ifndef SIMIT_GRAPH_H
#define SIMIT_GRAPH_H

#include <cassert>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <ostream>

#include "tensor_components.h"
#include "variadic.h"

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

class Function;

/// A Simit element reference.  All Simit elements live in Simit sets and an
/// ElementRef provides a reference to an element.
class ElementRef {
 private:
  explicit inline ElementRef(int ident) : ident(ident) {}
  int ident;

  friend SetBase;
  template <int cardinality> friend class Set;
  template <int cardinality> friend class hidden::EndpointIteratorBase;
  friend FieldRefBase;
};

// Base class for Sets
// Sets are used to represent collections within C++,
// and can be passed as bound inputs to Simit programs.
class SetBase {
public:
  SetBase() : elements(0), capacity(capacityIncrement) { }
  
  ~SetBase() {
    for (auto f: fields)
      delete f;
  }
  
  /// Return the number of elements in the Set
  int getSize() const { return elements; }

  /// Add a tensor field to the set.  Use the template parameters to specify the
  /// component type and dimension sizes of the tensors.  For example, define a
  /// field of 2x3 matrices containing doubles as follows:
  /// Field<double,2,3> matrix = addField<double,2,3>("mat");
  template <typename T, int... dimensions>
  FieldRef<T, dimensions...> addField(const std::string &name) {
    FieldData::TensorType *type =
        new FieldData::TensorType(typeOf<T>(), {dimensions...});
    FieldData *fieldData = new FieldData(type);
    fieldData->data = calloc(capacity, fieldData->sizeOfType);
    fields.push_back(fieldData);
    fieldNames[name] = fields.size()-1;
    return FieldRef<T, dimensions...>(fieldData);
  }
  
  /// Get a Field corresponding to the string fieldName
  template <typename T, int... dimensions>
  FieldRef<T, dimensions...> getField(std::string fieldName) {
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
      assert(isValidComponentType(f->type->getComponentType()));
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
    
    ElementIterator(SetBase* set, int idx=0) : curElem(idx), set(set) { }
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
    SetBase* set;           // set we're iterating over

    bool lessThan(const SetBase::ElementIterator &other) const {
      return this->curElem.ident < other.curElem.ident;
    }

    friend bool operator<(const SetBase::ElementIterator& e1,
                          const SetBase::ElementIterator& e2);
  };
  
  /// Create an ElementIterator for this Set, set to the first element
  ElementIterator begin() { return ElementIterator(this, 0); }
  
  /// Create an ElementIterator for terminating iteration over this Set
  ElementIterator end() { return ElementIterator(this, getSize()); }
  
protected:
  int elements;                      // number of elements in the set
  int capacity;                   // current capacity of the set
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
        assert(i<getOrder());
        return dimensions[i];
      }
      size_t getSize() const { return size; }
    private:
      ComponentType componentType;
      std::vector<int> dimensions;
      size_t size;
    };

    const TensorType *type;
    size_t sizeOfType;
    
    FieldData(const TensorType *type) : type(type), data(nullptr) {
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
  assert(e1.set == e2.set);
  return e1.lessThan(e2);
}

  
template <int cardinality=0>
class Set : public SetBase {
 public:
  template <typename ...T>
  Set(const T& ... sets) :  SetBase(), edge_data(nullptr) {
    assert(sizeof...(sets) == cardinality &&
           "Wrong number of endpoint sets");
   
    // TODO: this may not be the most efficient, but does it matter?
    endpointSets = epsMaker(endpointSets, sets...);
    
    edge_data = (int*) calloc(sizeof(int), capacity);
  }
  
  Set() : SetBase(), edge_data(nullptr) {
    assert(cardinality == 0 &&
           "Sets with cardinality>0 must provide sets for endpoints");
  }
  
  ~Set() {
    free(edge_data);
  }
  
  /// Add an edge.
  /// The endpoints refer to the respective Sets they come from.
  template <typename ...T>
  ElementRef addElement(T ... endpoints) {
    
    assert(sizeof...(endpoints) == cardinality &&
           "Wrong number of endpoints.");
    
    if (elements > capacity-1)
      increaseEdgeCapacity();
    
    addHelper(0, endpoints...);
    return SetBase::addElement();
  }
  
  ElementRef addElement() {
    assert(cardinality == 0 &&
           "Must provide endpoints for cardinality > 0");
    return SetBase::addElement();
  }
  
  /// Get an endpoint of an edge.
  ElementRef getEndpoint(ElementRef edge, int endpointNum) {
    // TODO: may want to use a pool of ElementRefs instead of creating
    // new ones each time
    return ElementRef(edge_data[edge.ident*cardinality+endpointNum]);
  }
  
  /// Iterator that iterates over the endpoints of an edge
  class EndpointIterator : public hidden::EndpointIteratorBase<cardinality> {
   public:
    EndpointIterator(Set<cardinality>* set, ElementRef elem,
                     int endpointN=0) : hidden::EndpointIteratorBase<cardinality>(set,
                                          elem, endpointN) { }
    
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
  int* edge_data;                           // container for edges
  std::vector<const SetBase*> endpointSets; // sets that the endpoints belong to
  
  void increaseEdgeCapacity() {
    edge_data = (int*)realloc(edge_data,
                              capacity+capacityIncrement*sizeof(int));
  }
  
  template <int c> friend class hidden::EndpointIteratorBase;
  
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
  void addHelper(int which, F f, T ... endpoints) {
    assert(endpointSets[which]->getSize() > f.ident &&
      "Invalid member of set in addEdge");
    edge_data[elements*cardinality+which] = f.ident;
    addHelper(which+1, endpoints...);
  }
  template <typename F>
  void addHelper(int which, F f) {
    assert(endpointSets[which]->getSize() > f.ident &&
    "Invalid member of set in addEdge");
    edge_data[elements*cardinality+which] = f.ident;
  }

};

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
      retElem.ident = set->edge_data[curElem.ident*cardinality+endpointNum];
    return *this;
  }
  
  EndpointIteratorBase operator++(int) {
    endpointNum++;
    if (endpointNum > cardinality-1)
      retElem.ident = -1;   // return invalid element
    else
      retElem.ident = set->edge_data[curElem.ident*cardinality+endpointNum];
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
  inline T *getElemDataPtr(ElementRef element, size_t elementFieldSize) {
    return &static_cast<T*>(data)[element.ident * elementFieldSize];
  }

 private:
  SetBase::FieldData *fieldData;
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
  inline T *getElemDataPtr(ElementRef element) {
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
  assert("General tensor operator<< not yet supported");
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

} // namespace simit

#endif

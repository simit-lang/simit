#ifndef SIMIT_GRAPH_H
#define SIMIT_GRAPH_H

#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <ostream>

#include "tensor_type.h"
#include "variadic.h"
#include "comparable.h"
#include "error.h"
#include "types.h"

namespace simit {

class Function;

class Set;
class FieldRefBase;
template <typename T, int... dimensions> class FieldRef;
template <typename T, int... dimensions> class TensorRef;

namespace internal {
class VertexToEdgeEndpointIndex;
class VertexToEdgeIndex;
class NeighborIndex;
}

namespace pe {
class SetEndpointPathIndex;
}

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

  friend std::ostream &operator<<(std::ostream &os, const ElementRef& er) {
    return os << er.ident;
  }

  int getIdent() const {return ident;}

private:
  explicit inline ElementRef(int ident) : ident(ident) {}

  int ident;

  friend class Set;
  friend class FieldRefBase;
  friend class internal::VertexToEdgeEndpointIndex;
  friend class internal::VertexToEdgeIndex;
  friend class internal::NeighborIndex;
  friend class pe::SetEndpointPathIndex;
};


// Base class for Sets
// Sets are used to represent collections within C++,
// and can be passed as bound inputs to Simit programs.
class Set {
public:
  Set(const std::string &name)
      : name(name), numElements(0), endpoints(nullptr),
        capacity(capacityIncrement), neighbors(nullptr) {}

  template <typename ...Sets>
  Set(const char *name, const Sets& ...sets) : Set(std::string(name)) {
    static_assert(util::areSame<Set, Sets...>{},
        "Set constructor takes an optional name followed by zero or more Sets");
    this->endpointSets = {&sets...};
    this->endpoints    = (int*)calloc(sizeof(int), capacity * getCardinality());
  }

  template <typename ...Sets>
  Set(std::string name, const Sets& ...sets) : Set(name.c_str(), sets...) {}

  template <typename ...Sets>
  Set(const Sets& ...sets) : Set("", sets...) {}

  ~Set() {
    for (auto f: fields) {
      delete f;
    }
    free(endpoints);
  }
  
  /// Return the number of elements in the Set
  inline int getSize() const { return numElements; }

  /// Return the number of endpoints of the elements in the set.  Non-edge sets
  /// have cardinality 0.
  inline int getCardinality() const { return endpointSets.size(); }

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
 
  // Added for reordering
  void setSpatialField(const std::string &name) {
    // need to check if the field actually exists because maps just add an entry
    // if none exists
    uassert(fieldNames.find(name) != fieldNames.end())
        << "Invalid field name setting spatial field";
    spatialFieldName = name;
  }

  /// Get a Field corresponding to the string fieldName
  template <typename T, int... dimensions>
  FieldRef<T, dimensions...> getField(std::string fieldName) {
    // need to check if the field actually exists because maps just add an entry
    // if none exists
    uassert(fieldNames.find(fieldName) != fieldNames.end())
        << "Invalid field name in getField()";
    FieldData *fieldData = fields[fieldNames[fieldName]];
    uassert(typeOf<T>() == fieldData->type->getComponentType())
        << "Incorrect field type.";
    return FieldRef<T, dimensions...>(fieldData);
  }

  /// Add a new element or edge, returning its handle.
  /// The endpoints refer to the respective Sets they come from.
  template <typename ...Endpoints>
  ElementRef add(Endpoints... endpoints) {
    iassert(sizeof...(endpoints) == getCardinality()) <<"Wrong number of endpoints.";
    if (numElements > capacity-1)
      increaseEdgeCapacity();
    
    addEndpoints(0, endpoints...);

    if (numElements > capacity-1)
      increaseCapacity();
    return ElementRef(numElements++);
  }

  /// Remove an element from the Set
  void remove(ElementRef element) {
    for (auto f : fields){
      switch (f->type->getComponentType()) {
        case ComponentType::Float: {
          float* data = (float*)f->data;
          data[element.ident] = data[numElements-1];
          break;
        }
        case ComponentType::Double: {
          double* data = (double*)f->data;
          data[element.ident] = data[numElements-1];
          break;
        }
        case ComponentType::Int: {
          int* data = (int*)f->data;
          data[element.ident] = data[numElements-1];
          break;
        }
        case ComponentType::Boolean: {
          bool* data = (bool*)f->data;
          data[element.ident] = data[numElements-1];
          break;
        }
        case ComponentType::FloatComplex: {
          float_complex* data = (float_complex*)f->data;
          data[element.ident] = data[numElements-1];
          break;
        }
        case ComponentType::DoubleComplex: {
          double_complex* data = (double_complex*)f->data;
          data[element.ident] = data[numElements-1];
          break;
        }
      }
    }
    numElements--;
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

    ElementIterator(const Set* set, int idx=0) : curElem(idx), set(set) { }
    ElementIterator(const ElementIterator& other) : curElem(other.curElem),
                                                    set(other.set) {}

    const ElementRef& operator*() const {return curElem;}
    const ElementRef* operator->() const {return &curElem;}

    ElementIterator& operator++() {
      curElem.ident++;
      return *this;
    }

    ElementIterator operator++(int) {
      curElem.ident++;
      return *this;
    }

    friend bool operator!=(const ElementIterator& l, const ElementIterator& r) {
      return !(l.set==r.set) || !(l.curElem == r.curElem);
    }

    friend bool operator==(const ElementIterator& l, const ElementIterator& r) {
      return (l.set==r.set) && (l.curElem == r.curElem);
    }

    friend bool operator<(const ElementIterator& l, const ElementIterator& r) {
      iassert(l.set == r.set);
      return l.curElem < r.curElem;
    }

    bool operator<(const ElementIterator& other) {
      iassert(set == other.set);
      return curElem.ident < other.curElem.ident;
    }

  private:
    ElementRef curElem; // current element index
    const Set* set; // set we're iterating over
  };

  /// Create an ElementIterator for this Set, set to the first element
  ElementIterator begin() const { return ElementIterator(this, 0); }
  
  /// Create an ElementIterator for terminating iteration over this Set
  ElementIterator end() const { return ElementIterator(this, getSize()); }

  /// Get the endpoint set at the given location.
  const Set *getEndpointSet(int loc) const {
    return endpointSets[loc];
  }

  /// A set is homogeneous of all it's endpoints come from the same set,
  /// otherwise it is heterogeneous.
  bool isHomogeneous() const {
    if (getCardinality() > 0) {
      const Set *firstEndpointSet = getEndpointSet(0);
      for (int i=1; i < getCardinality(); ++i) {
        // Endpointsets are the same if their pointers point at the same set
        if (getEndpointSet(i) != firstEndpointSet) {
          return false;
        }
      }
    }
    return true;
  }

  /// Get an endpoint of an edge
  ElementRef getEndpoint(ElementRef edge, int endpointNum) const {
    return ElementRef(endpoints[edge.ident*getCardinality() + endpointNum]);
  }
  
  class Endpoints {
  public:
    /// Iterator that iterates over the endpoints of an edge
    class Iterator {
    public:
      typedef std::input_iterator_tag iterator_category;
      typedef ElementRef value_type;
      typedef ptrdiff_t difference_type;

      Iterator(const Set *set, ElementRef elem, int endpointN=0)
          : curElem(elem), retElem(-1), endpointNum(endpointN), set(set) {
        if (set->getCardinality() > 0) {
          retElem = set->getEndpoint(curElem, endpointNum);
        }
      }
      Iterator(const Iterator& other)
          : curElem(other.curElem), retElem(other.retElem),
            endpointNum(other.endpointNum), set(other.set) { }

      const ElementRef& operator*() const {return retElem;}
      const ElementRef* operator->() const {return &retElem;}

      Iterator& operator++() {
        const int cardinality = set->getCardinality();
        endpointNum++;
        if (endpointNum > set->getCardinality()-1)
          retElem.ident = -1;   // return invalid element
        else
          retElem.ident = set->endpoints[curElem.ident*cardinality+endpointNum];
        return *this;
      }

      Iterator operator++(int) {
        const int cardinality = set->getCardinality();
        endpointNum++;
        if (endpointNum > cardinality-1)
          retElem.ident = -1;   // return invalid element
        else
          retElem.ident = set->endpoints[curElem.ident*cardinality+endpointNum];
        return *this;
      }

      friend bool operator!=(const Iterator &it1, const Iterator &it2) {
        return !(it1.set == it2.set) ||
               !(it1.curElem.getIdent() == it2.curElem.getIdent()) ||
               !(it1.endpointNum == it2.endpointNum);
      }

      friend bool operator==(const Iterator &it1, const Iterator &it2) {
        return (it1.set == it2.set) &&
               (it1.curElem.getIdent() == it2.curElem.getIdent()) &&
               (it1.endpointNum == it2.endpointNum);
      }

      friend inline bool operator<(const Iterator &e1,
                                   const Iterator &e2) {
        iassert(e1.set == e2.set)
            << "Comparing Endpoints::Iterators from two different Sets";
        iassert(e1.curElem.getIdent() == e2.curElem.getIdent())
            << "Comparing Endpoints::Iterators over two different edges";
        return e1.endpointNum < e2.endpointNum;
      }

    private:
      ElementRef curElem;  // current element index
      ElementRef retElem;  // element we're returning
      int endpointNum;     // the current endpoint number
      const Set *set;      // set we're iterating over
    };

    Endpoints(const Set *set, ElementRef edge) : set(set), edge(edge) {}

    Iterator begin() const {
      return Iterator(set, edge, 0);
    }

    Iterator end() const {
      return Iterator(set, edge, set->getCardinality());
    }

  private:
    const Set *set;
    ElementRef edge;
  };

  Endpoints getEndpoints(ElementRef edge) const {
    return Endpoints(this, edge);
  }

  void *getFieldData(const std::string &fieldName) {
    iassert(fieldNames.find(fieldName) != fieldNames.end());
    return fields[fieldNames.at(fieldName)]->data;
  }

  /// Get an array containing, for each edge in a set, the elements it connects.
  int *getEndpointsData() { return endpoints; }

  /// If this set is an edge set with cardinality 2 then return an index that
  /// for each element in the first connected set contains it's neighbors in the
  /// second connceted set. Otherwise, return nullptr.
  const internal::NeighborIndex *getNeighborIndex() const;

  void setName(const std::string &name) { this->name = name; }
  std::string getName() const { return name; }

  friend std::ostream &operator<<(std::ostream &os, const Set &set) {
    return set.streamOut(os);
  }

  // A field on the members of the Set.
  // Invariant: elements < capacity
  struct FieldData {
    // Replace with simit::TensorType
    class TensorType {
    public:
      TensorType(ComponentType componentType, std::initializer_list<int> dims)
          : componentType(componentType), dimensions(dims), size(1) {
        for (auto dim : dimensions) {
          size *= dim;
        }
      }
      TensorType(ComponentType componentType, std::vector<int> dims)
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

    FieldData(const std::string &name, const TensorType *type, Set *set)
        : name(name), type(type), set(set), data(nullptr) {
      sizeOfType = componentSize(type->getComponentType()) * type->getSize();
    }

    ~FieldData() {
      if (data != nullptr) free(data);
      delete type;
    }

    std::string name;
    
    const TensorType *type;
    size_t sizeOfType;

    // The Set this field is a member of. Used for printing, etc.
    Set *set;

    /// Buffer for the field data
    void* data;

    /// Field references so that we can update their data pointers if we realloc
    /// field data. Avoids two loads on field get/set.
    std::set<FieldRefBase*> fieldReferences;
    
  private:
    /// disable copy constructors
    FieldData(const FieldData& f);
    FieldData& operator=(const FieldData& f);
  };

  // Added getters for reordering
  inline int* getEndpointsPtr() { return endpoints; }
  inline int getFieldIndex(std::string name) { return fieldNames[name]; } 
  inline std::vector<FieldData*>& getFields() { return fields; } 
  inline std::string getSpatialFieldName() const { return spatialFieldName; }
  inline bool hasSpatialField() const { return !spatialFieldName.empty(); }

private:

  // Set data
  std::string name;
  std::string spatialFieldName;
  int numElements;                           // number of elements in the set
  std::vector<const Set*> endpointSets;      // the sets the endpoints belong to
  int* endpoints;                            // the endpoints of edge elements

  int capacity;                              // current capacity of the set
  static const int capacityIncrement = 1024; // increment for capacity increases

  mutable internal::NeighborIndex *neighbors;// neighbor index (lazily created)
  std::map<std::string, int> fieldNames;     // name to field lookups
  std::vector<FieldData*> fields;            // fields of elements in the set

  /// disable copy constructors
  Set(const Set& s);
  Set& operator=(const Set& s);

  /// increase capacity of all fields
  void increaseCapacity();

  /// helpers for constructing endpoint sets
  template <typename F, typename ...T> std::vector<const Set*>
  epsMaker(std::vector<const Set*> sofar, const F& f, const T& ... sets) const {
    sofar.push_back(&f);
    return epsMaker(sofar, sets...);
  }
  template <typename F> std::vector<const Set*>
  epsMaker(std::vector<const Set*> sofar, const F& f) const {
    sofar.push_back(&f);
    return sofar;
  }
  std::vector<const Set*>
  epsMaker(std::vector<const Set*> sofar) {return sofar;}

  void increaseEdgeCapacity() {
    size_t newSize = (capacity+capacityIncrement)*getCardinality()*sizeof(int);
    endpoints = (int*)realloc(endpoints, newSize);
  }

  // helper for adding edges
  template <typename F, typename ...T>
  void addEndpoints(int which, F f, T ... eps) {
    uassert(endpointSets[which]->getSize() > f.ident)
        << "Invalid member of set in addEdge";
    endpoints[numElements*getCardinality()+which] = f.ident;
    addEndpoints(which+1, eps...);
  }
  template <typename F>
  void addEndpoints(int which, F f) {
    uassert(endpointSets[which]->getSize() > f.ident)
        << "Invalid member of set in addEdge";
    endpoints[numElements*getCardinality()+which] = f.ident;
  }
  void addEndpoints(int) {}

  // helper for building a set based on IR type
  void buildSetFields(const ir::ElementType *type) {
    for (auto f : fields) {
      delete f;
    }

    for (const ir::Field& field : type->fields) {
      const ir::TensorType *ttype = field.type.toTensor();
      ComponentType ctype;
      switch (ttype->getComponentType().kind) {
        case ir::ScalarType::Int: {
          ctype = ComponentType::Int;
          break;
        }
        case ir::ScalarType::Float: {
          ctype = ComponentType::Float;
          break;
        }
        default: {
          not_supported_yet;
        }
      }
      std::vector<int> dims;
      for (const ir::IndexDomain &domain : ttype->getDimensions()) {
        dims.push_back(domain.getSize());
      }
      FieldData::TensorType *type =
          new FieldData::TensorType(ctype, dims);
      FieldData *fieldData = new FieldData(field.name, type, this);
      fieldData->data = calloc(capacity, fieldData->sizeOfType);
      fields.push_back(fieldData);
      fieldNames[field.name] = fields.size()-1;
    }
  }

  std::ostream &streamOut(std::ostream &os) const {
    os << "{";
    auto it = begin();
    auto it_end = end();
    if (it != it_end) {
      os << it->ident;
      if (getCardinality() > 0) {
        os << ":(";
        os << endpoints[0];
        for (int i=1; i<getCardinality(); ++i) {
          os << "," << endpoints[i];
        }
        os << ")";
      }
      ++it;
    }
    while (it != it_end) {
      os << ", " << it->ident;
      if (getCardinality() > 0) {
        os << ":(";
        os << endpoints[it->ident*getCardinality() + 0];
        for (int i=1; i<getCardinality(); ++i) {
          os << "," << endpoints[it->ident*getCardinality() + i];
        }
        os << ")";
      }
      ++it;
    }
    return os << "}";
  }

  friend FieldRefBase;
  friend simit::Function;
};


// Field References

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
      : fieldData(static_cast<Set::FieldData*>(fieldData)),
        data(this->fieldData->data) {
    this->fieldData->fieldReferences.insert(this);
  }

  template <typename T>
  inline T *getElemDataPtr(ElementRef element, size_t elementFieldSize) const {
    iassert(sizeof(T) == componentSize(fieldData->type->getComponentType()));
    return &static_cast<T*>(data)[element.ident * elementFieldSize];
  }

  Set::FieldData *fieldData;

private:
  void *data;

  friend Set;
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

  TensorRef<T, dimensions...> operator()(ElementRef element) {
    return get(element);
  }

  const TensorRef<T, dimensions...> operator()(ElementRef element) const {
    return get(element);
  }

  void set(ElementRef element, std::initializer_list<T> values) {
    iassert(values.size() == (TensorRef<T,dimensions...>::getSize()))
        << "Incorrect number of init values";
    T *elemData = this->getElemDataPtr(element);
    size_t i=0;
    for (T val : values) {
      elemData[i++] = val;
    }
  }
  
  void set(ElementRef element, std::vector<float> values) {
    iassert(values.size() == (TensorRef<T,dimensions...>::getSize()))
        << "Incorrect number of init values : " << (TensorRef<T,dimensions...>::getSize());
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

template <typename T, int... dimensions>
class FieldRef : public FieldRefBaseParameterized<T,dimensions...> {
 private:
  FieldRef(void *fieldData)
      : FieldRefBaseParameterized<T,dimensions...>(fieldData) {}
  friend class Set;

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

  friend std::ostream &operator<<(std::ostream &os, const FieldRef<T> &field) {
    os << "[";
    auto it = field.fieldData->set->begin();
    auto end = field.fieldData->set->end();
    if (it != end) {
      os << (T)field.get(*it);
      ++it;
    }

    while (it != end) {
      os << ", " << (T)field.get(*it);
      ++it;
    }

    return os << "]";
  }

 private:
  FieldRef(void *fieldData) : FieldRefBaseParameterized<T>(fieldData) {}
  friend class Set;
};
/// @endcond


// Tensor References

template <typename ComponentType, int... Dimensions>
class TensorRef
    : public interfaces::Comparable<TensorRef<ComponentType,Dimensions...>> {
 public:
  static size_t getOrder() {
    return sizeof...(Dimensions);
  }

  static size_t getSize() {return util::product<Dimensions...>::value;}

  inline TensorRef<ComponentType>& operator=(ComponentType val) {
    static_assert(sizeof...(Dimensions) == 0,
                  "Can only assign scalar values to scalar tensors.");
    data[0] = val;
    return *this;
  }

  inline TensorRef<ComponentType,Dimensions...>&
  operator=(const std::initializer_list<ComponentType> &vals) {
    iassert(vals.size() == util::product<Dimensions...>::value);
    size_t i=0;
    for (ComponentType val : vals) {
      data[i++] = val;
    }
    return *this;
  }

  inline operator ComponentType() const {
    static_assert(sizeof...(Dimensions) == 0,
                  "Can only convert scalar tensors to scalar values.");
    return data[0];
  }

  template <typename... Indices>
  inline ComponentType& operator()(Indices... index) {
    static_assert(sizeof...(index) == sizeof...(Dimensions),
                  "Incorrect number of indices used to index tensor");
    return data[util::computeOffset(util::seq<Dimensions...>(), index...)];
  }

  template <typename... Indices> inline
  const ComponentType& operator()(Indices... index) const {
    static_assert(sizeof...(index) == sizeof...(Dimensions),
                  "Incorrect number of indices used to index tensor");
    return data[util::computeOffset(util::seq<Dimensions...>(), index...)];
  }

  friend bool operator==(const TensorRef& l, const TensorRef& r){
    return l.data == r.data;
  }

  friend bool operator<(const TensorRef& l, const TensorRef& r){
    return l.data < r.data;
  }

 private:
  inline TensorRef(ComponentType *data) : data(data) {}
  ComponentType *data;

  friend class FieldRefBaseParameterized<ComponentType, Dimensions...>;
};

template <typename T, int... dims>
std::ostream &operator<<(std::ostream &os, const TensorRef<T, dims...> & t) {
  static_assert(sizeof...(dims) <= 2,
                "TensorRef operator<< only currently supported for order <= 2");
  return os;
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

  for (int i=1; i<r; ++i) {
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


// Graph generators
void createElements(Set *elements, unsigned num);

class Box {
public:
  typedef std::pair<ElementRef,ElementRef> Coord;

  Box(unsigned nX, unsigned nY, unsigned nZ, std::vector<ElementRef> refs,
      std::map<Box::Coord, ElementRef> coords2edges);

  unsigned numX() const {return nX;}
  unsigned numY() const {return nY;}
  unsigned numZ() const {return nZ;}

  ElementRef operator()(unsigned x, unsigned y, unsigned z) {
    return refs[z*nY*nX + y*nX + x];
  }

  ElementRef getEdge(ElementRef p1, ElementRef p2) const;

  std::vector<ElementRef> getEdges();

private:
  unsigned nX, nY, nZ;
  std::vector<ElementRef> refs;
  std::map<Coord, ElementRef> coords2edges;
};

Box createBox(Set *vertices, Set *edges,
              unsigned numX, unsigned numY, unsigned numZ);

} // namespace simit

#endif

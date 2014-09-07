#ifndef SIMIT_GRAPH_H
#define SIMIT_GRAPH_H

#include <cassert>
#include <vector>
#include <string>
#include <map>
#include <set>

#include "tensor.h"
#include "variadic.h"

namespace simit {

// Forward declarations
class Set;
class FieldRefBase;
template <typename T, int... dimensions> class FieldRef;
template <typename T, int... dimensions> class TensorRef;


/// A Simit element reference.  All Simit elements live in Simit sets and an
/// ElementRef provides a reference to an element.
class ElementRef {
 private:
  explicit ElementRef(int ident) : ident(ident) {}
  int ident;

  friend Set;
  friend FieldRefBase;
};

/// A Simit Set.
/// Sets are used to represent collections within C++,
/// and can be passed as bound inputs to Simit programs.
class Set {
 public:
  Set() : elements(0), capacity(capacityIncrement) { }
  
  ~Set() {
    for (auto f: fields)
      delete f;
  }
  
  /// Return the number of elements in the Set
  int getSize() { return elements; }

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
      switch (f->type->getComponentType()) {
        case Type::FLOAT: {
          double* data = (double*)f->data;
          data[element.ident] = data[elements-1];
          break;
        }
        case Type::INT: {
          int* data = (int*)f->data;
          data[element.ident] = data[elements-1];
          break;
        }
        case Type::ELEMENT:
          assert(false && "ELEMENT types not supported yet.");
          break;
        default:
          assert(false);
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
    
    ElementIterator(Set* set, int idx=0) : curElem(idx), set(set) { }
    ElementIterator(const ElementIterator& other) : curElem(other.curElem),
    set(other.set) { }
    
    friend inline bool operator<(const Set::ElementIterator& e1,
                                 const Set::ElementIterator& e2);
    
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
    Set* set;           // set we're iterating over

    bool lessThan(const Set::ElementIterator &other) const {
      return this->curElem.ident < other.curElem.ident;
    }

    friend bool operator<(const Set::ElementIterator& e1,
                          const Set::ElementIterator& e2);
  };
  
  /// Create an ElementIterator for this Set, set to the first element
  ElementIterator begin() { return ElementIterator(this, 0); }
  
  /// Create an ElementIterator for terminating iteration over this Set
  ElementIterator end() { return ElementIterator(this, getSize()); }
  
 private:
  // A field on the members of the Set.
  //
  // Invariant: elements < capacity
  struct FieldData {
    class TensorType {
    public:
      TensorType(Type componentType, std::initializer_list<int> dimensions)
      : componentType(componentType), dimensions(dimensions), size(1) {
        for (auto dim : dimensions) {
          size *= dim;
        }
      }
      Type getComponentType() const { return componentType; }
      size_t getOrder() const { return dimensions.size(); }
      size_t getDimension(size_t i) const {
        assert(i<getOrder());
        return dimensions[i];
      }
      size_t getSize() const { return size; }
    public:
      Type componentType;
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
  int elements;                      // number of elements in the set
  int capacity;                   // current capacity of the set
  static const int capacityIncrement = 1024; // increment for capacity increases
  
  // disable copy constructors
  Set(const Set& s);
  Set& operator=(const Set& s);

  // increase capacity of all fields
  void increaseCapacity();

  friend FieldRefBase;
};

inline bool operator<(const Set::ElementIterator& e1,
                      const Set::ElementIterator& e2) {
  assert(e1.set == e2.set);
  return e1.lessThan(e2);
}


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

  FieldRefBase (const FieldRefBase& other) {
    data = other.data;
    fieldData = other.fieldData;
    this->fieldData->fieldReferences.insert(this);
  }

  FieldRefBase (FieldRefBase&& other) {
    std::swap (data, other.data);
    std::swap (fieldData, other.fieldData);
    this->fieldData->fieldReferences.erase(&other);
    this->fieldData->fieldReferences.insert(this);
  }

  FieldRefBase& operator= (const FieldRefBase &other) {
    data = other.data;
    fieldData = other.fieldData;
    this->fieldData->fieldReferences.insert(this);
    return *this;
  }

  FieldRefBase& operator= (FieldRefBase&& other) {
    std::swap(data, other.data);
    std::swap (fieldData, other.fieldData);
    this->fieldData->fieldReferences.erase(&other);
    this->fieldData->fieldReferences.insert(this);
    return *this;
  }

 protected:
  FieldRefBase(void *fieldData)
      : fieldData(static_cast<Set::FieldData*>(fieldData)),
        data(this->fieldData->data) {
    this->fieldData->fieldReferences.insert(this);
  }

  template <typename T>
  inline T *getElemDataPtr(ElementRef element, size_t elementFieldSize) {
    return &static_cast<T*>(data)[element.ident * elementFieldSize];
  }

 private:
  Set::FieldData *fieldData;
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

template <typename T, int... dimensions>
class FieldRef : public FieldRefBaseParameterized<T,dimensions...> {
 private:
  FieldRef(void *fieldData)
      : FieldRefBaseParameterized<T,dimensions...>(fieldData) {}
  friend class Set;
};

template <typename T>
class FieldRef<T> : public FieldRefBaseParameterized<T> {
 public:
  void set(ElementRef element, T val) {
    (*this->getElemDataPtr(element)) = val;
  }

 private:
  FieldRef(void *fieldData) : FieldRefBaseParameterized<T>(fieldData) {}
  friend class Set;
};


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
  inline T &operator()(Indices... indices) {
    static_assert(sizeof...(dimensions) > 0,
                  "Access scalars directly, not through operator()");
    static_assert(sizeof...(indices) == sizeof...(dimensions),
                  "Incorrect number of indices used to index tensor");
    auto dims = simit::util::seq<dimensions...>();
    return data[simit::util::computeOffset(dims, indices...)];
  }

  template <typename... Indices>
  inline const T &operator()(Indices... indices) const {
    return operator()(indices...);
  }

 private:
  TensorRef(T *data) : data(data) {}
  T *data;

  friend class FieldRefBaseParameterized<T, dimensions...>;
};

} // namespace simit

#endif

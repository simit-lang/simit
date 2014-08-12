#ifndef SIMIT_GRAPH_H
#define SIMIT_GRAPH_H

#include <cassert>

#include "tensor.h"


namespace simit {

class Set {

};

/** A field on the members of the Set.
  *
  * Invariant: items < capacity
  */
class Field {
public:
  Type type;                    // simit type of data
  
  Field(Type type) : type(type) {
    //TODO: support ELEMENT
    if (type == Type::ELEMENT)
      assert(false && "Sets do not currently support ELEMENT types");
    if (type == Type::INT)
      data = (void*) calloc(capacityIncrement, sizeof(int));
    if (type == Type::FLOAT)
      data = (void*) calloc(capacityIncrement, sizeof(double));
    
    capacity = capacityIncrement;
    items = 0;
  }
  
  /** Add an item by adding a new field entry, setting it to the given value */
  template <typename T>
  void add(T val);
  
  /** Set the field of item idx to a new value */
  template <typename T>
  void set(const unsigned int idx, T val);
  
  /** Get the value of a field of an item */
  template <typename T>
  void get(const unsigned int idx, T* val);
  
  /** Remove an item, avoiding fragmentation. */
  void remove(const unsigned int idx);
  
private:
  void* data;                   // buffer for the data
  unsigned int capacity;        // current capacity
  unsigned int items;           // current number of items
  
  static const unsigned int capacityIncrement = 1024;
  
  void increaseCapacity();
};

  
namespace internal {
  

} // namespace internal
  
}

#endif

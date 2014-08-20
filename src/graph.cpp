#include "graph.h"

namespace simit {

/* Field */
  
void Field::increaseCapacity() {
  data = (void*) realloc(data, capacity + capacityIncrement);
}

template <>
void Field::set<int>(const int idx, int val) {
  assert((type == Type::INT) && "Setting a value with wrong type");
  assert((idx < items) && "Setting a value on a non-existent item");

  ((int*)data)[idx] = val;
}
  
template <>
void Field::set<double>(const int idx, double val) {
  assert((type == Type::FLOAT) && "Setting a value with wrong type");
  assert((idx < items) && "Setting a value on a non-existent item");
  
  ((double*)data)[idx] = val;
}

void Field::remove(const int idx) {
  // Should this be ok?
  assert((idx < items) && "Removing a non-existent item");
  
  // set the item at idx to the last value, and decrement
  // the number of items.  this should work as long as idx
  // is a valid index
  if (type == Type::INT)
    ((int*)data)[idx] = ((int*)data)[items-1];
  if (type == Type::FLOAT)
    ((double*)data)[idx] = ((double*)data)[items-1];

  items--;
}
  
template <>
void Field::get<int>(const int idx, int* val) {
  assert((type == Type::INT) && "Setting a value with wrong type");
  assert((idx < items) && "Setting a value on a non-existent item");
  
  *val = ((int*)data)[idx];
}
  
template <>
void Field::get<double>(const int idx, double* val) {
  assert((type == Type::FLOAT) && "Setting a value with wrong type");
  assert((idx < items) && "Setting a value on a non-existent item");
  
  *val = ((double*)data)[idx];
}
  
/* end Field */

  
/* Set */
ElementHandle Set::addItem() {
  for (auto f : fields) {
    if (f->type == Type::INT)
      f->add(0);
    else if (f->type == Type::FLOAT)
      f->add(0.0);
    else
      f->add(0);
  }
  return items++;
}

  
/* end Set */
}

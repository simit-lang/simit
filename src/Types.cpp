#include "Types.h"
#include <iostream>
#include <assert.h>

using namespace Simit;
using namespace std;


// Dimension
Dimension::Dimension()
: type(VARIABLE) {}

Dimension::Dimension(unsigned int size)
: type(ANONYMOUS), size(size) {}

Dimension::~Dimension() {}

Dimension::operator std::string() const {
  switch(type) {
    case VARIABLE:
      return "*";
    case ANONYMOUS:
      return to_string(size);
    case SET:
      assert(false); // Not supported yet
  }

  assert(false);
  return "";
}

namespace Simit {
  std::ostream& operator<<(std::ostream &out, const Dimension *dim) {
    return out << string(*dim);
  }
}
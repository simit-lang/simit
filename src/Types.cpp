#include "Types.h"

#include <assert.h>
#include <iostream>

#include "Util.h"

using namespace std;

namespace simit {
  std::ostream& operator<<(std::ostream &out, const Dimension *dim) {
    return out << string(*dim);
  }


  // Shape
  Shape::~Shape() {
    for (auto dim : dimensions) {
      delete dim;
    }
  }

  Shape::operator std::string() const {
    return "[" + util::join(dimensions, ", ") + "]";
  }


  // Dimension
  Dimension::operator std::string() const {
    switch (type) {
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
}

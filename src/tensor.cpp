#include "tensor.h"

#include <ostream>
#include <string>

#include "ir.h"
#include "types.h"
#include "util/compare.h"
#include "util/arrays.h"

using namespace simit::ir;

namespace simit {


// Compare function
bool compareTensors(const TensorType& ltype, const void *ldata,
                    const TensorType& rtype, const void *rdata) {
  if (ltype != rtype) {
    return false;
  }

  switch (ltype.getComponentType()) {
    case ComponentType::Float:
      return util::compare<float>(ldata, rdata, ltype.getSize());
    case ComponentType::Double:
      return util::compare<double>(ldata, rdata, ltype.getSize());
    case ComponentType::Int:
      return util::compare<int>(ldata, rdata, ltype.getSize());
    case ComponentType::Boolean:
      return util::compare<bool>(ldata, rdata, ltype.getSize());
    case ComponentType::FloatComplex:
      return util::compare<float_complex>(ldata, rdata, ltype.getSize());
    case ComponentType::DoubleComplex:
      return util::compare<double_complex>(ldata, rdata, ltype.getSize());
  }
  return false;
}

}

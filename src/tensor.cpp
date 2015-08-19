#include "tensor.h"

#include <ostream>
#include <string>

#include "ir.h"
#include "types.h"
#include "util/compare.h"

using namespace simit::ir;

namespace simit {

template <typename T>
static void zero(void *data, unsigned size) {
  T *tdata = static_cast<T*>(data);
  for (unsigned i=0; i < size; ++i) {
    tdata[i] = T();
  }
}

// class Tensor
struct Tensor::Content {
  ir::Type type;
  void *data;

  Content(const Type &type) {
    iassert(type.isTensor());
    this->type = type;

    unsigned numElements = getSize();
    this->data = malloc(numElements * getComponentSize());

    switch (type.toTensor()->componentType.kind) {
      case ir::ScalarType::Boolean:
        zero<bool>(data, numElements);
        break;
      case ir::ScalarType::Int:
        zero<int>(data, numElements);
        break;
      case ir::ScalarType::Float:
        if (ir::ScalarType::singleFloat()) {
          iassert(ir::ScalarType::floatBytes == sizeof(float));
          zero<float>(data, numElements);
        }
        else {
          iassert(ir::ScalarType::floatBytes == sizeof(double));
          zero<double>(data, numElements);
        }
        break;
    }
  }

  Content(const Type &type, void *data) {
    iassert(type.isTensor());
    this->type = type;

    unsigned dataSize = getSize() * getComponentSize();
    this->data = malloc(dataSize);
    memcpy(this->data, data, dataSize);
  }

  unsigned getSize() const {
    return type.toTensor()->size();
  }

  unsigned getComponentSize() const {
    return type.toTensor()->componentType.bytes();
  }
};

Tensor::Tensor(int val) : Tensor(Int, &val) {
}

Tensor::Tensor(double val) : Tensor(Float, &val) {
}

Tensor::Tensor(const ir::Type &type) : content(new Tensor::Content(type)) {
}

Tensor::Tensor(const ir::Type &type, void *data)
    : content(new Tensor::Content(type, data)) {
}

Tensor::Tensor(const Tensor &other)
    : content(new Tensor::Content(other.content->type, other.content->data)) {
}

Tensor::Tensor(Tensor &&other) noexcept : content(other.content) {
  other.content = nullptr;
}

Tensor &Tensor::operator=(const Tensor &other) {
  if (&other == this) {
    return *this;
  }
  Tensor tmp(other);
  *this = std::move(tmp);
  return *this;
}

Tensor &Tensor::operator=(Tensor &&other) noexcept {
  std::swap(content, other.content);
  return *this;
}

Tensor::~Tensor() {
  if (content != nullptr) {
    iassert(content->data != nullptr);
    free(content->data);
    delete content;
    content = nullptr;
  }
}

const ir::Type &Tensor::getType() const {
  return content->type;
}

unsigned Tensor::getSize() const {
  return content->getSize();
}

unsigned Tensor::getComponentSize() const {
  return content->getComponentSize();
}

void *Tensor::getData() {
  return content->data;
}

const void *Tensor::getData() const {
  return content->data;
}

bool operator==(const Tensor &l, const Tensor &r) {
  Type ltype = l.getType();
  Type rtype = r.getType();
  iassert(ltype.isTensor() && rtype.isTensor());

  if (ltype != rtype) {
    return false;
  }

  const void *ldata = l.getData();
  const void *rdata = r.getData();
  size_t size = l.getSize();
  switch (ltype.toTensor()->componentType.kind) {
    case ir::ScalarType::Int: {
      return util::compare<int>(ldata, rdata, size);
    }
    case ir::ScalarType::Float: {
      if (ir::ScalarType::floatBytes == sizeof(float)) {
        return util::compare<float>(ldata, rdata, size);
      }
      else if (ir::ScalarType::floatBytes == sizeof(double)) {
        return util::compare<double>(ldata, rdata, size);
      }
    }
    case ir::ScalarType::Boolean: {
      return util::compare<bool>(ldata, rdata, size);
    }
  }
}

bool operator!=(const Tensor &l, const Tensor &r) {
  return !(l == r);
}


template <typename T>
static void printData(std::ostream &os, const T *data, unsigned size) {
  if (size == 1) {
    os << data[0];
  }
  else {
    os << "[" << data[0];
    for (size_t i=1; i < size; ++i) {
      os << ", " << data[i];
    }
    os << "]";
  }
}

std::ostream &operator<<(std::ostream &os, const Tensor &tensor) {
  ir::Type type = tensor.getType();
  const void *data = tensor.getData();

  switch (type.kind()) {
    case ir::Type::Tensor: {
      const ir::TensorType *tensorType = type.toTensor();
      unsigned size = tensorType->size();
      ir::ScalarType::Kind componentType = tensorType->componentType.kind;

      switch (componentType) {
        case ir::ScalarType::Int:  {
          printData(os, static_cast<const int*>(data), size);
          break;
        }
        case ir::ScalarType::Float: {
          if (ir::ScalarType::floatBytes == sizeof(float)) {
            printData(os, static_cast<const float*>(data), size);
          }
          else if (ir::ScalarType::floatBytes == sizeof(double)) {
            printData(os, static_cast<const double*>(data), size);
          }
          else {
            ierror;
          }
          break;
        }
        case ir::ScalarType::Boolean: {
          printData(os, static_cast<const bool*>(data), size);
          break;
        }
      }
    }
      break;
    case ir::Type::Element:
    case ir::Type::Set:
    case ir::Type::Tuple:
      not_supported_yet;
      break;
  }
  return os;
}


// Compare function
bool tensorsCompare(const TensorType& ltype, const void *ldata,
                    const TensorType& rtype, const void *rdata) {
  if (ltype != rtype) {
    return false;
  }

  switch (ltype.getComponentType()) {
    case ComponentType::Int: {
      return util::compare<int>(ldata, rdata, ltype.getSize());
    }
    case ComponentType::Float: {
      if (ir::ScalarType::floatBytes == sizeof(float)) {
        return util::compare<float>(ldata, rdata, ltype.getSize());
      }
      else if (ir::ScalarType::floatBytes == sizeof(double)) {
        return util::compare<double>(ldata, rdata, ltype.getSize());
      }
    }
    case ComponentType::Boolean: {
      return util::compare<bool>(ldata, rdata, ltype.getSize());
    }
  }
}

}

#include "tensor.h"

#include <ostream>
#include <string>

#include "ir.h"
#include "types.h"

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

  switch (ltype.toTensor()->componentType.kind) {
    case ir::ScalarType::Int: {
      const int *ldata = static_cast<const int*>(l.getData());
      const int *rdata = static_cast<const int*>(r.getData());
      for (size_t i=0; i < l.getSize(); ++i) {
        if (ldata[i] != rdata[i])
          return false;
      }
      break;
    }
    case ir::ScalarType::Float: {
      if (ir::ScalarType::floatBytes == sizeof(float)) {
        const float *ldata = static_cast<const float*>(l.getData());
        const float *rdata = static_cast<const float*>(r.getData());
        for (size_t i=0; i < l.getSize(); ++i) {
          if (!util::almostEqual(ldata[i], rdata[i], 0.0001f)) {
            return false;
          }
        }
      }
      else if (ir::ScalarType::floatBytes == sizeof(double)) {
        const double *ldata = static_cast<const double*>(l.getData());
        const double *rdata = static_cast<const double*>(r.getData());
        for (size_t i=0; i < l.getSize(); ++i) {
          if (!util::almostEqual(ldata[i], rdata[i], 0.0001)) {
            return false;
          }
        }
      }
      break;
    }
    case ir::ScalarType::Boolean: {
      const bool *ldata = static_cast<const bool*>(l.getData());
      const bool *rdata = static_cast<const bool*>(r.getData());
      for (size_t i=0; i < l.getSize(); ++i) {
        if (ldata[i] != rdata[i])
          return false;
      }
      break;
    }

  }
  return true;
}

bool operator!=(const Tensor &l, const Tensor &r) {
  Type ltype = l.getType();
  Type rtype = r.getType();

  if (ltype != rtype) {
    return true;
  }

  switch (ltype.toTensor()->componentType.kind) {
    case ir::ScalarType::Int: {
      const int *ldata = static_cast<const int*>(l.getData());
      const int *rdata = static_cast<const int*>(r.getData());
      for (size_t i=0; i < l.getSize(); ++i) {
        if (ldata[i] == rdata[i])
          return false;
      }
      break;
    }
    case ir::ScalarType::Float: {
      if (ir::ScalarType::floatBytes == sizeof(float)) {
        const float *ldata = static_cast<const float*>(l.getData());
        const float *rdata = static_cast<const float*>(r.getData());
        for (size_t i=0; i < l.getSize(); ++i) {
          if (util::almostEqual(ldata[i], rdata[i], 0.001f)) {
            return false;
          }
        }
      }
      else if (ir::ScalarType::floatBytes == sizeof(double)) {
        const double *ldata = static_cast<const double*>(l.getData());
        const double *rdata = static_cast<const double*>(r.getData());
        for (size_t i=0; i < l.getSize(); ++i) {
          if (util::almostEqual(ldata[i], rdata[i], 0.001)) {
            return false;
          }
        }
      }
      break;
    }
    case ir::ScalarType::Boolean: {
      const bool *ldata = static_cast<const bool*>(l.getData());
      const bool *rdata = static_cast<const bool*>(r.getData());
      for (size_t i=0; i < l.getSize(); ++i) {
        if (ldata[i] == rdata[i])
          return false;
      }
      break;
    }
  }
  return true;
}


template <typename T>
static void printData(std::ostream &os, const T *data, unsigned size) {
  if (size == 1) {
    os << data[0];
  }
  else {
    os << "[" << data[0];
    for (size_t i=0; i < size; ++i) {
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


// class TensorType
const TensorType* TensorType::getBlockType() const {
  iassert(isBlocked());
  return blockType;
}

ComponentType TensorType::getComponentType() const {
  iassert(!isBlocked());
  return componentType;
}

size_t TensorType::getSize() const {
  size_t size = (isBlocked()) ? blockType->getSize() : 1;
  for (int dimension : dimensions) {
    size *= dimension;
  }
  return size;
}

std::ostream& operator<<(std::ostream& os, const TensorType& tensorType) {
  size_t order = tensorType.getOrder();
  os << "tensor";
  if (order > 0) {
    os << "[";
    os << tensorType.getDimension(0);
    for (size_t i=1; i < tensorType.getOrder(); ++i) {
      os << "," << tensorType.getDimension(i);
    }
    os << "]";
  }
  os << "(";
  if (tensorType.isBlocked()) {
    os << tensorType.getBlockType();
  }
  // Base case
  else {
    os << tensorType.getComponentType();
  }
  os << ")";
  return os;
}


}

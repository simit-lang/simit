#include "tensor_data.h"

#include "tensor.h"
#include "tensor_type.h"
#include "types.h"

namespace simit {

struct TensorData::Content {
  // TODO: Replace with non-ptr
  TensorType* type = nullptr;

  void* data     = nullptr;
  bool  ownsData = false;
};

TensorData::TensorData() : content(new Content) {
}

TensorData::TensorData(const TensorType& tensorType, void* data) : TensorData(){
  content->type = new TensorType(tensorType);
  content->data = data;
}

TensorData::TensorData(simit::Tensor* tensor) : TensorData() {
  content->data = tensor->getData();
  content->ownsData = false;
}

TensorData::~TensorData() {
  if (content->ownsData) {
    free(content->data);
  }
}

void* TensorData::getData() {
  return content->data;
}

const void* TensorData::getData() const {
  return content->data;
}

const TensorType& TensorData::getTensorType() const {
  return *content->type;
}

}

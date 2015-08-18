#include "tensor_data.h"

#include "tensor.h"
#include "tensor_type.h"
#include "types.h"

namespace simit {

struct TensorData::Content {
  TensorType* tensorType = nullptr;

  void* data     = nullptr;
  bool  ownsData = false;
};

TensorData::TensorData() : content(new Content) {
}

TensorData::TensorData(simit::Tensor* tensor) : TensorData() {
  content->data = tensor->getData();
  content->ownsData = false;
}

TensorData::TensorData(const simit::Tensor& tensor) : TensorData() {
  // TODO: Fix and test
//  data = malloc(tensor.getSizeInBytes());
  content->ownsData = true;
}

TensorData::~TensorData() {
  delete content->tensorType;
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
  return *content->tensorType;
}

}

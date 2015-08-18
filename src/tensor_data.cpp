#include "tensor_data.h"

#include "tensor.h"
#include "types.h"

namespace simit {

struct TensorData::Content {
  void* data     = nullptr;
  bool  ownsData = false;
};

TensorData::TensorData() : content(new Content) {
}

TensorData::TensorData(void* data, bool ownData) : TensorData() {
  content->data = data;
  content->ownsData = ownData;
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

}

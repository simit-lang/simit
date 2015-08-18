#ifndef SIMIT_TENSOR_DATA_H
#define SIMIT_TENSOR_DATA_H

#include <cstdlib>
#include <memory>

namespace simit {
class Tensor;
class TensorType;

class TensorData {
public:
  TensorData();
  TensorData(simit::Tensor* tensor);
  TensorData(const simit::Tensor& tensor);

  ~TensorData();

  void* getData();
  const void* getData() const;

  const TensorType& getTensorType() const;

private:
  struct Content;
  std::unique_ptr<Content> content;
};

}
#endif

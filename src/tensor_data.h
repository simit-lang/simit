#ifndef SIMIT_TENSOR_DATA_H
#define SIMIT_TENSOR_DATA_H

#include <cstdlib>
#include <memory>

namespace simit {
class Tensor;
class TensorType;

/// The TensorData class is an untyped version of simit::Tensor, where the types
/// are stored in a runtime TensorType object. This means it can be passed to
/// the backend where TensorData objects of different types are stored together.
class TensorData {
public:
  TensorData();
  TensorData(const TensorType& tensorType, void* data);
  TensorData(simit::Tensor* tensor);

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

#ifndef SIMIT_TENSOR_DATA_H
#define SIMIT_TENSOR_DATA_H

#include <cstdlib>
#include <memory>

namespace simit {

/// The TensorData class is an untyped version of simit::Tensor, where the types
/// are stored in a runtime TensorType object. This means it can be passed to
/// the backend where TensorData objects of different types are stored together.
class TensorData {
public:
  TensorData();
  TensorData(void* data, bool ownData=false);
  ~TensorData();

  void* getData();
  const void* getData() const;

private:
  // TODO: Remove indirection and move to backend_function.h
  struct Content;
  std::unique_ptr<Content> content;
};

}
#endif

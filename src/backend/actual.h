#ifndef SIMIT_ACTUAL_H
#define SIMIT_ACTUAL_H

#include "uncopyable.h"

namespace simit {
namespace backend {

class Actual : public simit::interfaces::Uncopyable {
public:
  Actual(bool output) : output(output) {
  }

  ~Actual() {
    if (bound && !output && kind == Tensor) {
      free(tensorData);
    }
  }

  void bindTensor(void* data) {
    kind = Tensor;
    this->tensorData = data;
  }

  void bindSet(simit::Set* set) {
    kind = Set;
    this->set = set;
  }

  bool isBound() const {return bound;}

  void setOutput(bool output) {this->output = output;}
  bool isOutput() const {return output;}

  const ir::Type& getType() const;

  simit::Set* getSet() {
    iassert(set != nullptr && kind == Set);
    return set;
  }

  void* getTensorData() {
    iassert(tensorData != nullptr && kind == Tensor);
    return tensorData;
  }

private:
  bool bound = false;

  enum Kind {Set, Tensor};
  Kind kind;
  union {
    simit::Set* set;
    void* tensorData;
  };

  /// Output actuals are references to Sets/Tensors in the user program. These
  /// must not be deleted.
  bool output;
};

}}
#endif

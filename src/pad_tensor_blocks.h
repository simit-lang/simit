#ifndef SIMIT_PAD_TENSOR_BLOCKS_H
#define SIMIT_PAD_TENSOR_BLOCKS_H

#include "hir.h"
#include "hir_visitor.h"

namespace simit {
namespace hir {

class PadTensorBlocks : public HIRVisitor {
public:
  void pad(Program::Ptr program) { program->accept(this); }

private:
  virtual void visit(NDTensorType::Ptr type) {
    padBlocks(type, getMaxDim(type));
  }

  unsigned getMaxDim(const TensorType::Ptr);
  void padBlocks(TensorType::Ptr, const unsigned);
};

}
}

#endif


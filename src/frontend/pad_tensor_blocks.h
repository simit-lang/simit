#ifndef SIMIT_PAD_TENSOR_BLOCKS_H
#define SIMIT_PAD_TENSOR_BLOCKS_H

#include "hir.h"
#include "hir_visitor.h"

namespace simit {
namespace hir {

// Experimental pass for padding tensor type dimensions in order to support 
// tensor types containing blocks of different order. Disabled for now since 
// current type system is too strict for this to be useful.
class PadTensorBlocks : public HIRVisitor {
public:
  void pad(Program::Ptr program) { program->accept(this); }

private:
  virtual void visit(NDTensorType::Ptr type) {
    padBlocks(type, getMaxDim(type));
  }

  static unsigned getMaxDim(TensorType::Ptr);
  
  void padBlocks(TensorType::Ptr, unsigned);
};

}
}

#endif


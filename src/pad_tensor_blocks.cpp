#include <vector>
#include <memory>
#include <algorithm>

#include "pad_tensor_blocks.h"
#include "hir.h"
#include "error.h"

namespace simit {
namespace hir {

unsigned PadTensorBlocks::getMaxDim(const TensorType::Ptr type) {
  if (!isa<NDTensorType>(type)) {
    return 0;
  }

  const auto tensorType = to<NDTensorType>(type);
  const unsigned maxDim = getMaxDim(tensorType->blockType);
  return std::max(maxDim, (unsigned)tensorType->indexSets.size());
}

void PadTensorBlocks::padBlocks(TensorType::Ptr type, const unsigned maxDim) {
  if (!isa<NDTensorType>(type)) {
    return;
  }

  const auto tensorType = to<NDTensorType>(type);
  padBlocks(tensorType->blockType, maxDim);
  
  if (tensorType->indexSets.size() == maxDim) {
    return;
  }

  std::vector<IndexSet::Ptr> indexSets;
  iassert(maxDim > tensorType->indexSets.size());
  for (unsigned i = 0; i < maxDim - tensorType->indexSets.size(); ++i) {
    const auto is = std::make_shared<RangeIndexSet>();
    is->range = 1;
    indexSets.push_back(is);
  }
  for (const auto &is : tensorType->indexSets) {
    indexSets.push_back(is);
  }
  tensorType->indexSets = indexSets;
}

}
}


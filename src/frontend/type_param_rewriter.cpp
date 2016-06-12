#include <memory>
#include <string>
#include <unordered_set>

#include "type_param_rewriter.h"
#include "hir.h"
#include "ir.h"
#include "hir_rewriter.h"

namespace simit {
namespace hir {

void TypeParamRewriter::visit(FuncDecl::Ptr decl) {
  if (decl->typeParams.size() == 0) {
    node = decl;
    return;
  }

  std::unordered_set<std::string> genericIndexSets;
  for (const auto set : decl->typeParams) {
    genericIndexSets.insert(set->setName);
  }

  class MarkGenerics : public HIRRewriter {
    public:
      MarkGenerics(const std::unordered_set<std::string> &genericIndexSets)
          : genericIndexSets(genericIndexSets) {}

    private:
      virtual void visit(SetIndexSet::Ptr set) {
        if (genericIndexSets.find(set->setName) != genericIndexSets.end()) {
          auto newSet = std::make_shared<GenericIndexSet>();
          newSet->setLoc(set);
          newSet->setName = set->setName;
          node = newSet;
        } else {
          node = set;
        }
      }

      std::unordered_set<std::string> genericIndexSets;
  };

  node = MarkGenerics(genericIndexSets).rewrite(decl);
}

}
}


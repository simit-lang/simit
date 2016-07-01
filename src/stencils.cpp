#include "stencils.h"

#include "ir.h"
#include <iostream>

using namespace std;

namespace simit {
namespace ir {

std::string StencilLayout::getStencilFunc() const {
  return ptr->assemblyFunc;
}

std::string StencilLayout::getStencilVar() const {
  return ptr->targetVar;
}

map<vector<int>, int> StencilLayout::getLayout() const {
  return ptr->layout;
}

Expr StencilLayout::getLatticeSet() const {
  iassert(ptr->latticeSet.defined());
  return ptr->latticeSet;
}

std::ostream& operator<<(std::ostream& os, const StencilLayout& stencil) {
  os << "stencil(" << stencil.getLatticeSet() << ")" << endl;
  if (stencil.defined()) {
    for (auto &kv : stencil.getLayout()) {
      os << "\t";
      bool first = true;
      for (int off : kv.first) {
        if (!first) os << ",";
        first = false;
        os << off;
      }
      os << ": " << kv.second << endl;
    }
  }
  os << "DONE" << endl;
}

}} // namespace simit::ir

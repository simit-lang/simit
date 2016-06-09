#include "stencils.h"

#include "ir.h"
#include <iostream>

using namespace std;

namespace simit {
namespace ir {

map<vector<int>, int> Stencil::getLayout() const {
  return ptr->layout;
}

Expr Stencil::getLatticeSet() const {
  iassert(ptr->latticeSet.defined());
  return ptr->latticeSet;
}

std::ostream& operator<<(std::ostream& os, const Stencil& stencil) {
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

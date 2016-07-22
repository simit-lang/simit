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

map<int, vector<int>> StencilLayout::getLayoutReversed() const {
  map<vector<int>, int> &layout = ptr->layout;
  map<int, vector<int>> reversed;
  for (auto &kv : layout) {
    reversed[kv.second] = kv.first;
  }
  return reversed;
}

Var StencilLayout::getLatticeSet() const {
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
  return os;
}

vector<int> getOffsets(vector<Expr> offsets) {
  vector<int> out;
  for (Expr off : offsets) {
    iassert(isa<Literal>(off));
    out.push_back(to<Literal>(off)->getIntVal(0));
  }
  return out;
}

StencilContent* buildStencil(Func kernel, Var stencilVar, Var latticeSet) {
  std::map<vector<int>, int> layout; // layout of stencil for the storage
  int stencilSize = 0;
  Var tensorVar;
  match(kernel,
        function< void(const TensorWrite*,Matcher*) >(
            [&](const TensorWrite* op, Matcher* ctx) {
              tensorVar = Var();
              ctx->match(op->tensor);
              // Found a write to the stencil-assembled variable
              if (tensorVar.defined() && tensorVar == stencilVar) {
                tassert(op->indices.size() == 2)
                    << "Stencil assembly must be of matrix";
                auto row = op->indices[0];
                auto col = op->indices[1];
                iassert(row.type().isElement() &&
                        col.type().isElement());
                iassert(kernel.getArguments().size() >= 3)
                    << "Kernel must have element, and two sets as arguments";
                // The first argument to the kernel is an alias for points[0,0,...]
                Var origin = kernel.getArguments()[0];
                Var points = kernel.getArguments()[kernel.getArguments().size()-1];
                Var links = kernel.getArguments()[kernel.getArguments().size()-2];
                iassert(points.getType().isSet());
                iassert(links.getType().isSet());
                iassert(links.getType().toSet()->kind == SetType::LatticeLink);
                int dims = links.getType().toSet()->dimensions;
                // We assume row index normalization has been performed already
                iassert((isa<VarExpr>(row) && to<VarExpr>(row)->var == origin) ||
                        (isa<SetRead>(row) && util::isAllZeros(
                            getOffsets(to<SetRead>(row)->indices))));
                // col index determines stencil structure
                vector<int> offsets;
                if (isa<VarExpr>(col)) {
                  iassert(to<VarExpr>(col)->var == origin);
                  offsets = vector<int>(dims);
                }
                else {
                  iassert(isa<SetRead>(col));
                  offsets = getOffsets(to<SetRead>(col)->indices);
                }
                // Add new offset to stencil
                if (!layout.count(offsets)) {
                  layout[offsets] = stencilSize;
                  stencilSize++;
                }
              }
            }),
        function< void(const VarExpr*) >(
            [&tensorVar](const VarExpr* v) {
              tensorVar = v->var;
            })
        );
  StencilContent *content = new StencilContent;
  content->layout = layout;
  content->latticeSet = latticeSet;
  return content;
}

}} // namespace simit::ir

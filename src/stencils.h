#ifndef SIMIT_STENCILS_H
#define SIMIT_STENCILS_H

#include <iostream>
#include <map>
#include <vector>

#include "var.h"
#include "intrusive_ptr.h"

using namespace std;

namespace simit {
namespace ir {
class Expr;

struct StencilContent {
  map<vector<int>, int> layout;
  // HACK: Store as a Var so we don't run into declaration loops with Expr
  Var latticeSet;

  mutable long ref = 0;
  friend inline void aquire(const StencilContent *v) {++v->ref;}
  friend inline void release(const StencilContent *v) {if (--v->ref==0) delete v;}
};

class Stencil : public util::IntrusivePtr<StencilContent> {
public:
  Stencil() : util::IntrusivePtr<StencilContent>() {}
  Stencil(StencilContent *ptr)
      : util::IntrusivePtr<StencilContent>(ptr) {}

  map<vector<int>, int> getLayout() const;
  Expr getLatticeSet() const;
};

std::ostream& operator<<(std::ostream&, const Stencil&);

}} // namespace simit::ir


#endif

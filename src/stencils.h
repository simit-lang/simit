#ifndef SIMIT_STENCILS_H
#define SIMIT_STENCILS_H

#include <iostream>
#include <map>
#include <vector>

#include "var.h"
#include "intrusive_ptr.h"
#include "util/collections.h"

using namespace std;

namespace simit {
namespace ir {
class Expr;
class Func;

struct StencilContent {
  map<vector<int>, int> layout;
  // HACK: Store as a Var so we don't run into declaration loops with Expr
  Var gridSet;

  // Unresolved Stencil form: just a reference to assembly func and out var
  std::string assemblyFunc;
  std::string targetVar;

  mutable long ref = 0;
  friend inline void aquire(const StencilContent *v) {++v->ref;}
  friend inline void release(const StencilContent *v) {if (--v->ref==0) delete v;}
};

class StencilLayout : public util::IntrusivePtr<StencilContent> {
public:
  StencilLayout() : util::IntrusivePtr<StencilContent>() {}
  StencilLayout(StencilContent *ptr)
      : util::IntrusivePtr<StencilContent>(ptr) {}

  /// Unresolved intermediate queries
  std::string getStencilFunc() const;
  std::string getStencilVar() const;

  /// Fully resolved form of layout
  map<vector<int>, int> getLayout() const;
  map<int, vector<int>> getLayoutReversed() const;

  bool hasGridSet() const;
  Var getGridSet() const;
};

std::ostream& operator<<(std::ostream&, const StencilLayout&);

/// Helper function for manipulating offset lists
vector<int> getOffsets(vector<Expr> offsets);

/// Analyze kernel function to build a stencil layout
StencilContent* buildStencil(Func kernel, Var stencilVar, Var gridSet);

}} // namespace simit::ir


#endif

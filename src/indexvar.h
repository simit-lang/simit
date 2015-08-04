#ifndef SIMIT_INDEXVAR_H
#define SIMIT_INDEXVAR_H

#include "intrusive_ptr.h"
#include "domain.h"
#include "reduction.h"

namespace simit {
namespace ir {

// namespace { // hiding disabled for now - GCC hates this, TODO: migrate inside *.cpp?
  struct IndexVarContent {
    std::string name;
    IndexDomain domain;
    ReductionOperator rop;
    Expr *fixedExpr;
    int kind;

    ~IndexVarContent();
    mutable long ref = 0;
    friend inline void aquire(IndexVarContent *c) {++c->ref;}
    friend inline void release(IndexVarContent *c) {if (--c->ref==0) delete c;}
  };
// }

/// An index variable describes iteration over an index set.  There are two
/// types of index variables, free index variables and reduction index
/// variables and both types are represented by the IndexVar class.
///
/// Free index variables simply describe iteration across an index set and do
/// not have a reduction operation (op=FREE).
///
/// Reduction variables have an associated reduction operation that is
/// performed for each index in the index set.  Examples are SUM, which not
/// surprisingly sums over the index variable (\sum_{i} in latex speak) and
/// product which takes the product over the index variable (\prod_{i}).
class IndexVar : public util::IntrusivePtr<IndexVarContent> {
public:
  /// The kind of an index variable.
  enum Kind { Free, Reduction, Fixed };

  /// Construct a free index variable.
  IndexVar(std::string name, IndexDomain domain)
      : IntrusivePtr(new IndexVarContent) {
    ptr->name = name;
    ptr->domain = domain;
    ptr->rop = ReductionOperator::Undefined;
    ptr->kind = Free;
    ptr->fixedExpr = nullptr;
  }
  
  // Construct a reduction index variable.
  IndexVar(std::string name, IndexDomain domain, ReductionOperator rop)
      : IntrusivePtr(new IndexVarContent) {
    ptr->name = name;
    ptr->domain = domain;
    ptr->rop = rop;
    ptr->kind = Reduction;
    ptr->fixedExpr = nullptr;
  }
  
  // Construct a fixed index variable
  IndexVar(std::string name, IndexDomain domain, Expr* expr)
      : IntrusivePtr(new IndexVarContent) {
    ptr->name = name;
    ptr->kind = Fixed;
    ptr->fixedExpr = expr;
    ptr->domain = domain;
  }


  std::string getName() const {return ptr->name;}
  const IndexDomain &getDomain() const {return ptr->domain;}
  Expr* getFixedExpr() const { return ptr->fixedExpr; }

  size_t getNumBlockLevels() const {return ptr->domain.getNumIndexSets();}

  bool isFreeVar() const { return  (ptr->kind == Free); }
  bool isReductionVar() const { return (ptr->kind == Reduction); }
  bool isFixed() const { return (ptr->kind == Fixed); }

  ReductionOperator getOperator() const { return ptr->rop; }
};

std::ostream &operator<<(std::ostream &os, const IndexVar &);

}} // namespace simit::ir

#endif

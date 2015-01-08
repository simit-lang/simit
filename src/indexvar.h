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
  /// Construct a free index variable.
  IndexVar(std::string name, IndexDomain domain)
      : IntrusivePtr(new IndexVarContent) {
    ptr->name = name;
    ptr->domain = domain;
    ptr->rop = ReductionOperator::Undefined;
  }

  // Construct a reduction index variable.
  IndexVar(std::string name, IndexDomain domain, ReductionOperator rop)
      : IntrusivePtr(new IndexVarContent) {
    ptr->name = name;
    ptr->domain = domain;
    ptr->rop = rop;
  }

  std::string getName() const {return ptr->name;}
  const IndexDomain &getDomain() const {return ptr->domain;}

  size_t getNumBlockLevels() const {return ptr->domain.getNumBlockLevels();}

  bool isFreeVar() const { return  !isReductionVar(); }
  bool isReductionVar() const {
    return ptr->rop.getKind() != ReductionOperator::Undefined;
  }

  ReductionOperator getOperator() const { return ptr->rop; }
};

std::ostream &operator<<(std::ostream &os, const IndexVar &);

}} // namespace simit::ir

#endif

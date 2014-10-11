#ifndef SIMIT_INDEXVAR_H
#define SIMIT_INDEXVAR_H

#include "domain.h"
#include "reductions.h"

namespace simit {
namespace ir {

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
class IndexVar {
public:
  /// Construct a free index variable.
  IndexVar(std::string name, IndexDomain domain)
      : name(name), domain(domain), reduction(false) {}

  // Construct a reduction index variable.
  IndexVar(std::string name, IndexDomain domain, ReductionOperator rop)
      : name(name), domain(domain), reduction(true), rop(rop) {}

  std::string getName() const { return name; }

  const IndexDomain &getDomain() const { return domain; }

  bool isFreeVar() const { return !reduction; }
  bool isReductionVar() const { return reduction; }

  ReductionOperator getOperator() const { return rop; }

private:
  std::string name;
  IndexDomain domain;

  bool reduction;
  ReductionOperator rop;
};

std::ostream &operator<<(std::ostream &os, const IndexVar &var);

}} // namespace simit::ir

#endif

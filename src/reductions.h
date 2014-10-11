#ifndef SIMIT_REDUCTIONS_H
#define SIMIT_REDUCTIONS_H

#include <string>
#include <ostream>

namespace simit {
namespace ir {

/// A Simit reduction operator can be used with index expressions or maps.
/// Since reductions happen over unordered sets, the reduction operators must
/// be both associative and commutative. Supported reduction operators are:
/// - Sum
class ReductionOperator {
public:
  enum Kind { Sum };  // TODO: Add Poduct, Max, Min, and user-defined functions

  // Construct an undefiend reduction operator.
  ReductionOperator() : kind(Sum) {}

  // Construct a reduction operator.
  ReductionOperator(Kind kind) : kind(kind) {}
  
  Kind getKind() const {return kind;}

  /// Returns the name of the reduction variable (e.g. sum).
  std::string getName();

private:
  Kind kind;
};

std::ostream &operator<<(std::ostream &os, const ReductionOperator &);

}} // namespace simit::ir

#endif

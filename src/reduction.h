#ifndef SIMIT_REDUCTION_H
#define SIMIT_REDUCTION_H

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
  // TODO: Add Poduct, Max, Min, and user-defined functions
  enum Kind { Sum, Undefined };

  // Construct an undefiend reduction operator.
  ReductionOperator() : kind(Undefined) {}

  // Construct a reduction operator.
  ReductionOperator(Kind kind) : kind(kind) {}
  
  Kind getKind() const {return kind;}

  /// Returns the name of the reduction variable (e.g. sum).
  std::string getName();

private:
  Kind kind;
};

bool operator==(const ReductionOperator &, const ReductionOperator &);
bool operator!=(const ReductionOperator &, const ReductionOperator &);
std::ostream &operator<<(std::ostream &os, const ReductionOperator &);

}}

#endif

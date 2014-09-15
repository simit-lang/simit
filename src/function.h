#ifndef SIMIT_FUNCTION_H
#define SIMIT_FUNCTION_H

#include <cstdlib>
#include <vector>
#include <map>

#include "interfaces.h"

// TODO: Remove
#include "ir.h"

namespace simit {
namespace ir {
class Function;
class Argument;
class Literal;
class Type;
}

// TODO: Replace with a simple tensor implementation
//typedef std::shared_ptr<simit::internal::Literal> TensorPtr;
typedef simit::ir::Literal Tensor;
class SetBase;

class Function : public simit::interfaces::Printable,
                        simit::interfaces::Uncopyable {
public:
  virtual ~Function();

  void bind(const std::string &argName, Tensor *tensor);
  void bind(const std::string &argName, SetBase *set);

  inline void run() {
    if (initRequired) {
      funcPtr = init(actuals);
      initRequired = false;
    }
    funcPtr();
  }

protected:
  typedef void (*FuncPtrType)();
  class Actual {
  public:
    Actual(const std::shared_ptr<ir::Type> &type = NULL) : type(type) {
      val.tensor = NULL;
    }
    void bind(Tensor *tensor) { val.tensor = tensor; }
    void bind(SetBase *set) { val.set = set; }
    bool isBound() const { return val.tensor != NULL; }
    const ir::Type *getType() const { return type.get(); }
    const Tensor *getTensor() { return val.tensor; }
    const SetBase *getSet() { return val.set; }
  private:
    std::shared_ptr<ir::Type> type;
    union {
      SetBase *set;
      Tensor  *tensor;
    } val;
  };
  
  Function(const ir::Function &simitFunc);

private:
  std::map<std::string, Actual> actuals;

  FuncPtrType funcPtr;
  bool initRequired;
  virtual FuncPtrType init(std::map<std::string, Actual> &actuals) = 0;
};

} // namespace simit
#endif

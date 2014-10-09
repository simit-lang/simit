#ifndef SIMIT_FUNCTION_H
#define SIMIT_FUNCTION_H

#include <cstdlib>
#include <vector>
#include <map>

#include "interfaces.h"

// TODO: Remove
#include "ir.h"
#include "types.h"


namespace simit {
namespace ir {
class Function;
class Argument;
class Literal;
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
      funcPtr = init(formals, actuals);
      initRequired = false;
    }
    funcPtr();
  }

protected:
  typedef void (*FuncPtrType)();
  class Actual {
  public:
    Actual(const ir::Type &type) : type(type), tensor(NULL) {}
    Actual() : Actual(ir::Type()) {}
    void bind(Tensor *tensor) { this->tensor = tensor; }
    void bind(SetBase *set) { this->set = set; }
    bool isBound() const { return tensor != NULL; }
    const ir::Type &getType() const { return type; }
    Tensor *getTensor() { assert(tensor); return tensor; }
    SetBase *getSet() { assert(set); return set; }
  private:
    ir::Type type;
    union {
      SetBase *set;
      Tensor  *tensor;
    };
  };
  
  Function(const ir::Function &simitFunc);

  void *getFieldPtr(const SetBase *base, const std::string &fieldName);

private:
  std::vector<std::string> formals;
  std::map<std::string, Actual> actuals;

  FuncPtrType funcPtr;
  bool initRequired;
  virtual FuncPtrType init(const std::vector<std::string> &formals,
                           std::map<std::string, Actual> &actuals) = 0;
};

} // namespace simit
#endif

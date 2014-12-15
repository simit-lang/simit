#ifndef SIMIT_FUNCTION_H
#define SIMIT_FUNCTION_H

#include <cstdlib>
#include <vector>
#include <map>

#include "printable.h"
#include "uncopyable.h"

// TODO: Remove
#include "ir.h"
#include "types.h"


namespace simit {
namespace ir {
class Func;
struct Literal;
}

// TODO: Replace with a simple tensor implementation
typedef simit::ir::Expr Tensor;
class SetBase;

class Function : public simit::interfaces::Printable,
                        simit::interfaces::Uncopyable {
public:
  virtual ~Function();

  void bind(const std::string &argName, Tensor *tensor);
  void bind(const std::string &argName, SetBase *set);

  inline void init() {
    funcPtr = init(formals, actuals);
    initRequired = false;
  }

  inline void runSafe() {
    if (initRequired) {
      init();
    }
    funcPtr();
  }

  inline void run() {
    iassert(!initRequired);
    funcPtr();
  }

protected:
  typedef void (*FuncPtrType)();
  typedef std::function<void()> FuncType;
  class Actual {
  public:
    Actual(const ir::Type &type) : type(type), tensor(NULL) {}
    Actual() : Actual(ir::Type()) {}
    void bind(Tensor *tensor) { this->tensor = tensor; }
    void bind(SetBase *set) { this->set = set; }
    bool isBound() const { return tensor != NULL; }
    const ir::Type &getType() const { return type; }
    Tensor *getTensor() { iassert(tensor); return tensor; }
    SetBase *getSet() { iassert(set); return set; }
  private:
    ir::Type type;
    union {
      SetBase *set;
      Tensor  *tensor;
    };
  };
  
  Function(const ir::Func &simitFunc);

private:
  std::vector<std::string> formals;
  std::map<std::string, Actual> actuals;

  FuncType funcPtr;
  bool initRequired;
  virtual FuncType init(const std::vector<std::string> &formals,
                        std::map<std::string, Actual> &actuals) = 0;
};

} // namespace simit
#endif

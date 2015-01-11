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
    unmapArgs();
    funcPtr();
    mapArgs();
  }

  inline void run() {
    iassert(!initRequired);
    funcPtr();
  }
  
  // TODO Should these really be an extension to the bind interface?
  //      Per-argument updates/copies.
  //      Don't always write in a new pointer (requires re-JIT), just alert to
  //      updates in pointed-to data.
  virtual void mapArgs() {}
  virtual void unmapArgs(bool updated=true) {}

protected:
  typedef void (*FuncPtrType)();
  typedef std::function<void()> FuncType;
  class Actual {
  public:
    Actual(const ir::Type &type, bool output=false)
      : type(type), tensor(NULL), output(output) {}
    Actual() : Actual(ir::Type()) {}
    void bind(Tensor *tensor) { this->tensor = tensor; }
    void bind(SetBase *set) { this->set = set; }
    bool isBound() const { return tensor != NULL; }
    void setOutput(bool output) { this->output = output; }
    bool isOutput() const { return output; }
    const ir::Type &getType() const { return type; }
    Tensor *getTensor() { iassert(tensor != nullptr); return tensor; }
    SetBase *getSet() { iassert(set != nullptr); return set; }
  private:
    ir::Type type;
    union {
      SetBase *set;
      Tensor  *tensor;
    };
    bool output;
  };
  
  Function(const ir::Func &simitFunc);

  std::vector<std::string> formals;
  std::map<std::string, Actual> actuals;

  /// We store the simit Function's literals to prevent their memory from being
  /// reclaimed, as compiled functions are expected to access them at runtime.
  std::vector<simit::ir::Expr> literals;

  FuncType funcPtr;
  bool initRequired;

private:
  virtual FuncType init(const std::vector<std::string> &formals,
                        std::map<std::string, Actual> &actuals) = 0;
};

} // namespace simit
#endif

#ifndef SIMIT_FUNCTION_H
#define SIMIT_FUNCTION_H

#include <cstdlib>
#include <vector>
#include <map>

#include "printable.h"
#include "uncopyable.h"
#include "tensor.h"

namespace simit {

namespace ir {
class Func;
class TensorStorage;
}

class SetBase;

namespace internal {

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

  bool isInit() {
    return !initRequired;
  }

  inline void run() {
    iassert(!initRequired);
    funcPtr();
  }

  inline void runSafe() {
    if (initRequired) {
      init();
    }
    unmapArgs();
    funcPtr();
    mapArgs();
  }
  
  // TODO Should these really be an extension to the bind interface?
  //      Per-argument updates/copies.
  //      Don't always write in a new pointer (requires re-JIT), just alert to
  //      updates in pointed-to data.
  virtual void mapArgs() {}
  virtual void unmapArgs(bool updated=true) {}

  std::function<void()> getFunctionHandle() {return funcPtr;}

protected:
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

  /// Get number of bytes required to store tensors with `type` and `storage`.
  size_t size(const ir::TensorType &type,
              const ir::TensorStorage &storage) const;

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

}}
#endif

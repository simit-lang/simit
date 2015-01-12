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
class TensorStorage;
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
    Tensor *getTensor() { iassert(tensor != nullptr); return tensor; }
    SetBase *getSet() { iassert(set != nullptr); return set; }
  private:
    ir::Type type;
    union {
      SetBase *set;
      Tensor  *tensor;
    };
  };

  /// Get number of bytes required to store tensors with `type` and `storage`.
  size_t size(const ir::TensorType &type,
              const ir::TensorStorage &storage) const;

  Function(const ir::Func &simitFunc);

private:
  std::vector<std::string> formals;
  std::map<std::string, Actual> actuals;

  /// We store the simit Function's literals to prevent their memory from being
  /// reclaimed, as compiled functions are expected to access them at runtime.
  std::vector<simit::ir::Expr> literals;

  FuncType funcPtr;
  bool initRequired;
  virtual FuncType init(const std::vector<std::string> &formals,
                        std::map<std::string, Actual> &actuals) = 0;
};

} // namespace simit
#endif

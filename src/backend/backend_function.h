#ifndef SIMIT_BACKEND_FUNCTION_H
#define SIMIT_BACKEND_FUNCTION_H

#include <cstdlib>
#include <vector>
#include <map>

#include "printable.h"
#include "uncopyable.h"
#include "types.h"

namespace simit {

class Set;
class Tensor;

namespace ir {
class Func;
class TensorStorage;
}

namespace backend {

class TensorData {
public:
  TensorData() : data(nullptr), ownsData(false) {}
  TensorData(simit::Tensor* tensor);
  TensorData(const simit::Tensor& tensor);

  ~TensorData() {
    if (ownsData) {
      free(data);
    }
  }

  void* getData() {return data;}
  const void* getData() const {return data;}

private:
  void* data;
  bool ownsData;
};

class Function : public simit::interfaces::Printable,
                        simit::interfaces::Uncopyable {
public:
  virtual ~Function();

  void bind(const std::string &argName, simit::Tensor *tensor);
  void bind(const std::string &argName, simit::Set *set);

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

  /// Print the function as machine code.
  virtual void printMachine(std::ostream &os) const = 0;

protected:
  typedef std::function<void()> FuncType;

  class Actual {
  public:
    Actual(const ir::Type &type, bool output=false)
        : type(type), bound(false), output(output) {}
    Actual() : Actual(ir::Type()) {}

    ~Actual() {
      if (bound && type.isTensor()) {
        delete tensor;
      }
    }

    void bind(simit::Tensor* tensor) {this->tensor = new TensorData(tensor);}
    void bind(simit::Set* set) {this->set = set;}

    bool isBound() const {return bound;}

    void setOutput(bool output) {this->output = output;}
    bool isOutput() const {return output;}

    const ir::Type& getType() const {return type;}

    simit::Set* getSet() {
      iassert(set != nullptr && type.isSet());
      return set;
    }

    TensorData* getTensor() {
      iassert(tensor != nullptr && type.isTensor());
      return tensor;
    }

  private:
    ir::Type type;

    bool bound;
    union {
      simit::Set* set;
      TensorData* tensor;
    };

    /// Output actuals are references to Sets/Tensors in the user program. These
    /// must not be deleted.
    bool output;
  };

  /// Get number of bytes required to store tensors with `type` and `storage`.
  size_t size(const ir::TensorType &type,
              const ir::TensorStorage &storage) const;

  Function(const ir::Func &simitFunc);

  std::vector<std::string> formals;
  std::map<std::string, Actual> actuals;

  /// We store the Simit Function's literals to prevent their memory from being
  /// reclaimed if the IR is deleted, as compiled functions are expected to
  /// access them at runtime.
  std::vector<simit::ir::Expr> literals;

  FuncType funcPtr;
  bool initRequired;

private:
  virtual FuncType init(const std::vector<std::string> &formals,
                        std::map<std::string, Actual> &actuals) = 0;
};

}}
#endif

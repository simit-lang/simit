#ifndef SIMIT_BACKEND_FUNCTION_H
#define SIMIT_BACKEND_FUNCTION_H

#include <cstdlib>
#include <vector>
#include <map>
#include <memory>

#include "printable.h"
#include "uncopyable.h"
#include "error.h"

namespace simit {

class Set;

namespace ir {
class Func;
class Expr;
class TensorStorage;
class Type;
struct TensorType;
}

namespace backend {

class Actual : public simit::interfaces::Uncopyable {
public:
  Actual(const ir::Type &type, bool output=false);
  Actual();
  ~Actual();

  void bindTensorData(void* data);
  void bind(simit::Set* set);

  bool isBound() const;

  void setOutput(bool output);
  bool isOutput() const;

  const ir::Type& getType() const;

  simit::Set* getSet();
  void* getTensorData();

private:
  struct Content;
  std::unique_ptr<Content> content;
};

class Function : public simit::interfaces::Printable,
                        simit::interfaces::Uncopyable {
public:
  virtual ~Function();

  /// Bind the given data to the argument with the given argName. The data is
  /// assumed to be laid out in the way specified by the type, which is
  /// type-checked against the formal.
  void bindTensorData(const std::string &argumentName, const ir::Type& type,
                      void* data);

  /// Bind the given data to the argument with the given argName. The data is
  /// assumed to be laid out in the format specified in the IR and is not
  /// type-checked.
  void bindTensorData(const std::string& argumentName, void* data);

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

  Function(const ir::Func &simitFunc);

  /// Get number of bytes required to store tensors with `type` and `storage`.
  size_t size(const ir::TensorType &type,
              const ir::TensorStorage &storage) const;

  std::vector<std::string> formals;
  std::map<std::string, Actual*> actuals;

  /// We store the Simit Function's literals to prevent their memory from being
  /// reclaimed if the IR is deleted, as compiled functions are expected to
  /// access them at runtime.
  std::vector<simit::ir::Expr> literals;

  FuncType funcPtr;
  bool initRequired;

private:
  virtual FuncType init(const std::vector<std::string> &formals,
                        const std::map<std::string, Actual*> &actuals) = 0;
};

}}
#endif

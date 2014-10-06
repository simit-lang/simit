#ifndef SIMIT_IR_PRINTER_H
#define SIMIT_IR_PRINTER_H

#include "ir_visitors.h"

#include <ostream>
#include <map>
#include <set>
#include <memory>

namespace simit {
namespace ir {

class Expression;
class IndexedTensor;

std::ostream &operator<<(std::ostream &os, const Function &);
std::ostream &operator<<(std::ostream &os, const Expression &);

class IRPrinter : public IRConstVisitor {
public:
  IRPrinter(std::ostream &os, signed indent=0);
  virtual ~IRPrinter() {}

  void print(const Function &);
  void print(const Expression &);
  void print(const IndexedTensor &);

private:
  virtual void handle(const Argument *);
  virtual void handle(const Result *);
  virtual void handle(const Literal *);
  virtual void handle(const IndexExpr *);
  virtual void handle(const Call *);
  virtual void handle(const FieldRead *);
  virtual void handle(const FieldWrite *);
  virtual void handle(const TensorRead *);
  virtual void handle(const TensorWrite *);

  void indent();
  std::string getName(const Expression *);
  std::string getName(const std::shared_ptr<Expression> &);

  std::ostream &os;
  unsigned indentation;
  bool printingFunctionBody;

  std::map<const IRNode *,std::string> nodeToName;
  std::set<std::string> names;
  signed id;
};

}} // namespace simit::ir
#endif

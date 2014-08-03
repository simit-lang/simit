#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

namespace simit {

class IRNode;
class Merge;

class IRVisitor {
 public:
  virtual ~IRVisitor();
  virtual void visit(const Merge *);
};


class IRPrinter : public IRVisitor {
 public:


};

}
#endif

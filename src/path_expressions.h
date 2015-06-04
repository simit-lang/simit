#ifndef SIMIT_PATH_EXPRESSIONS_H
#define SIMIT_PATH_EXPRESSIONS_H

#include "printable.h"
#include "intrusive_ptr.h"

namespace simit {
class Set;
namespace pe {

class PathExpressionImpl : public interfaces::Printable {
public:
  virtual ~PathExpressionImpl() {}
  virtual const simit::Set *getPathEndpoint(unsigned pathEndpoint) const = 0;

  mutable long ref = 0;
  friend inline void aquire(PathExpressionImpl *p) {++p->ref;}
  friend inline void release(PathExpressionImpl *p) {if (--p->ref==0) delete p;}
};

class EV : public PathExpressionImpl {
public:
  EV(simit::Set *E, simit::Set *V, unsigned edgeEndpoint);

  const simit::Set *getPathEndpoint(unsigned pathEndpoint) const;

private:
  simit::Set *E;
  simit::Set *V;
  unsigned edgeEndpoint;

  void print(std::ostream &os) const;
};

class VE : public PathExpressionImpl {
public:
  VE(simit::Set *V, simit::Set *E, unsigned edgeEndpoint);

  const simit::Set *getPathEndpoint(unsigned pathEndpoint) const;

private:
  simit::Set *V;
  simit::Set *E;
  unsigned edgeEndpoint;

  void print(std::ostream &os) const;
};

class PathExpression : public util::IntrusivePtr<PathExpressionImpl> {
public:
  PathExpression() : IntrusivePtr() {}
  PathExpression(PathExpressionImpl *impl) : IntrusivePtr(impl) {}

  const simit::Set *getPathEndpoint(unsigned pathEndpoint) const;
};
std::ostream &operator<<(std::ostream&, const PathExpression&);


}}

#endif

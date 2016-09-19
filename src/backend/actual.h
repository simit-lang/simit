#ifndef SIMIT_ACTUAL_H
#define SIMIT_ACTUAL_H

#include "interfaces/uncopyable.h"

namespace simit {
class Set;

namespace backend {

class SetActual;
class TensorActual;

class ActualVisitor {
public:
  virtual void visit(SetActual* actual) = 0;
  virtual void visit(TensorActual* actual) = 0;
};

class Actual : public simit::interfaces::Uncopyable {
public:
  virtual ~Actual() {}
  virtual void accept(ActualVisitor* v) = 0;
};

class SetActual : public Actual {
public:
  SetActual(simit::Set* set) : set(set) {}
  virtual ~SetActual() {}

  const simit::Set* getSet() const {return set;}
  simit::Set* getSet() {return set;}

  virtual void accept(ActualVisitor* v) {v->visit(this);}

private:
  simit::Set* set;  // TODO: Change to not depend on simit::Set
};

class TensorActual : public Actual {
public:
  TensorActual(void* data) : data(data) {}
  virtual ~TensorActual() {}

  const void* getData() const {return data;}
  void* getData() {return data;}

  virtual void accept(ActualVisitor* v) {v->visit(this);}

private:
  void* data;
};

class SparseTensorActual : public TensorActual {
public:
  SparseTensorActual(const unsigned* coords, const unsigned* sinks, void* data)
      : TensorActual(data), coords(coords), sinks{sinks} {}
  virtual ~SparseTensorActual() {}

  const unsigned* getCoords() const {return coords;}
  const unsigned* getSinks() const {return sinks;}

  virtual void accept(ActualVisitor* v) {v->visit(this);}

private:
  const unsigned* coords;
  const unsigned* sinks;
};

template <typename A>
inline bool isa(Actual* a) {
  return dynamic_cast<const A*>(a) != nullptr;
}

template <typename A>
inline A* to(Actual* a) {
  iassert(isa<A>(a)) << "Wrong Actual type";
  return static_cast<A*>(a);
}

}}
#endif

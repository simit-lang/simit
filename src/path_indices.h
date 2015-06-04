#ifndef SIMIT_PATH_INDICES_H
#define SIMIT_PATH_INDICES_H

#include <ostream>
#include <map>
#include "printable.h"
#include "graph.h"

namespace simit {
namespace pe {
class PathExpression;
class PathIndexBuilder;
class PathIndexImpl;

class PathIndex {
public:
  class Sinks;

  PathIndex();

  bool defined();

  Sinks getSinks(ElementRef element);

private:
  std::shared_ptr<PathIndexImpl> impl;
  PathIndex(PathIndexImpl *impl);
  friend PathIndexBuilder;
};

class PathIndexBuilder {
public:
  PathIndexBuilder() {}
  PathIndex build(const PathExpression &pe, unsigned sourceEndpoint);

private:
  std::map<PathExpression,PathIndex> pathIndices;
};


class PathIndexImpl : public interfaces::Printable {
public:
  virtual ~PathIndexImpl() {}
};

class CSRPathIndex : public PathIndexImpl {
public:
private:
  void print(std::ostream &os) const;
};

class PathIndex::Sinks {

};

}}

#endif

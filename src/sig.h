#ifndef SIMIT_SIG_H
#define SIMIT_SIG_H

#include <vector>
#include <map>
#include <set>
#include <iostream>

#include "types.h"
#include "indexvar.h"

namespace simit {
namespace ir {

struct SIGEdge;
class SIGVisitor;

struct SIGVertex {
  IndexVar iv;
  std::vector<SIGEdge*> connectors;

  SIGVertex(const IndexVar &iv) : iv(iv) {}
};
std::ostream &operator<<(std::ostream &os, const SIGVertex &);

struct SIGEdge {
  std::string name;
  std::vector<SIGVertex*> endpoints;

  SIGEdge(const std::string &name, const std::vector<SIGVertex*> &endpoints)
      : name(name), endpoints(endpoints) {
    for (auto v : endpoints) {
      v->connectors.push_back(this);
    }
  }
};
std::ostream &operator<<(std::ostream &os, const SIGEdge &);

class SIG {
public:
  enum MergeOp { Union, Intersection };

  explicit SIG() : content(new SIG::Content) {}
  explicit SIG(const IndexVar &iv);
  explicit SIG(std::string name, const std::vector<IndexVar> &ivs);

  friend SIG merge(SIG&, SIG&, SIG::MergeOp);

private:
  struct Content {
    std::map<IndexVar, std::unique_ptr<SIGVertex>> vertices;
    std::map<std::string, std::unique_ptr<SIGEdge>> edges;
  };
  std::shared_ptr<Content> content;

  friend SIGVisitor;
};

std::ostream &operator<<(std::ostream &os, const SIG &);


class SIGVisitor {
public:
  virtual void apply(const SIG &sig);
  virtual void apply(const SIG &sig, const IndexVar &first);

protected:
  std::set<const SIGVertex*> visitedVertices;
  std::set<const SIGEdge*> visitedEdges;

  virtual void visit(const SIGVertex *v);
  virtual void visit(const SIGEdge *e);
};

}} // namespace
#endif

#include "path_indices.h"

#include <iostream>
#include "path_expressions.h"
#include "graph.h"

namespace simit {
namespace pe {

// class PathIndex
PathIndex::PathIndex() : impl(nullptr) {
}

PathIndex::PathIndex(PathIndexImpl *impl) : impl(impl) {
}

bool PathIndex::defined() {
  return this->impl != nullptr;
}

PathIndex::Sinks PathIndex::getSinks(ElementRef element) {
  return Sinks();
}

// class PathIndexBuilder
PathIndex PathIndexBuilder::build(const PathExpression &pe,
                                  unsigned sourceEndpoint) {
  PathIndexImpl *pathIndex = nullptr;

  return PathIndex(pathIndex);
}


// class CSRPathIndex
void CSRPathIndex::print(std::ostream &os) const {
  os << "CSRPathIndex";
}

}}

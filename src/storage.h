#ifndef SIMIT_STORAGE_H
#define SIMIT_STORAGE_H

#include "ir_visitor.h"

#include <cstdlib>
#include <vector>
#include <map>
#include <memory>

namespace simit {
namespace ir {
  struct IRNode;
}}

namespace simit {
namespace internal {

class Storage {
public:
  Storage(int size) : size(size) { data = malloc(size); }
  ~Storage() { free(data); }

  void *getData() { return data; }
  int getSize() const { return size; }

private:
  void *data;
  int size;
};


class TemporaryAllocator : ir::IRVisitor {
public:
  TemporaryAllocator() {}

  std::map<ir::IRNode*, void*> allocateTemporaries(ir::Function *f);
  std::vector<std::shared_ptr<Storage>> getTemporaries() const {return storage;}

private:
  std::vector<std::shared_ptr<Storage>> storage;
  std::map<ir::IRNode*, void*>          temps;
};

}} // namespace simit::internal
#endif

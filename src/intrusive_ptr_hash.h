#ifndef SIMIT_INTRUSIVE_PTR_HASH_H
#define SIMIT_INTRUSIVE_PTR_HASH_H

#include <functional>

#include "intrusive_ptr.h"
#include "ir.h"

namespace std {

template <typename T>
struct hash<simit::util::IntrusivePtr<T>> {
  size_t operator()(const simit::util::IntrusivePtr<T>& x) const {
    return (size_t) x.ptr;
  }
};

#define HASH_INTRUSIVE_PTR(type) template <>\
  struct hash<type> {\
    size_t operator()(const type& x) const {\
      return (size_t) x.ptr;\
    }\
  };

// SFINAE on detecting base class doesn't work with clang, need to wait for C++17. Workaround here

HASH_INTRUSIVE_PTR(simit::ir::Var)
HASH_INTRUSIVE_PTR(simit::ir::IndexVar)
HASH_INTRUSIVE_PTR(simit::ir::Expr)
HASH_INTRUSIVE_PTR(simit::ir::Stmt)
HASH_INTRUSIVE_PTR(simit::ir::Func)

#undef HASH_INTRUSIVE_PTR

}

#endif

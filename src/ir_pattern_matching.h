#ifndef SIMIT_IR_PATTERN_MATCHING_H
#define SIMIT_IR_PATTERN_MATCHING_H

#include "ir.h"
#include <numeric>
#include <functional>
#include <type_traits>

namespace simit {
namespace ir {

struct HandleCallbackArguments {
  template <typename T>
  static bool call(const T* node, std::nullptr_t) {
    return true;
  }

#if __cplusplus < 201700L
  template <typename T, typename U, typename =
                typename std::enable_if<std::is_same<bool,
                    typename std::result_of<U(const T*)>::type>::value>::type>
  static bool call(const T* node, U&& func) {
    return func(node);
  }

  template <typename T, typename U, typename =
                typename std::enable_if<std::is_same<void,
                    typename std::result_of<U(const T*)>::type>::value>::type>
  static bool call(const T* node, U&& func, int* dummy = 0) {
    func(node);
    return true;
  }
#else
  template <typename T, typename U, typename =
                typename std::enable_if<std::is_same<bool,
                    typename std::invoke_result<U(const T*)>::type>::value>::type>
  static bool call(const T* node, U&& func) {
    return func(node);
  }

  template <typename T, typename U, typename =
                typename std::enable_if<std::is_same<void,
                    typename std::invoke_result<U(const T*)>::type>::value>::type>
  static bool call(const T* node, U&& func, int* dummy = 0) {
    func(node);
    return true;
  }
#endif
};

template <typename T>
class PatternMatch;

template <typename T>
class PatternMatch<T*> : public PatternMatch<T> {
};

template <>
class PatternMatch<void> {
public:
  template <typename ExprOrStmt, typename Callback>
  static bool match(ExprOrStmt expr, Callback&&) {
    return true;
  }
};

#define GENERATE_PATTERNMATCH_0(TYPE)\
  template <>\
  class PatternMatch<TYPE> {\
  public:\
    template <typename ExprOrStmt>\
    static bool match(ExprOrStmt expr, std::nullptr_t) {\
      return isa<TYPE>(expr);\
    }\
    \
    template <typename ExprOrStmt>\
    static bool match(ExprOrStmt expr, const std::tuple<std::nullptr_t>&) {\
      return isa<TYPE>(expr);\
    }\
    \
    template <typename ExprOrStmt, typename Callback>\
    static bool match(ExprOrStmt expr, const std::tuple<Callback>& callbacks) {\
      if (isa<TYPE>(expr)) {\
        const TYPE* node = to<TYPE>(expr);\
        return HandleCallbackArguments::call(node, std::get<0>(callbacks));\
      }\
      return false;\
    }\
  };

#define GENERATE_PATTERNMATCH_1(TYPE, MEMBER1)\
  template <typename T1>\
  class PatternMatch<TYPE(T1)> {\
  public:\
    template <typename ExprOrStmt>\
    static bool match(ExprOrStmt expr, std::nullptr_t) {\
      return isa<TYPE>(expr);\
    }\
    \
    template <typename ExprOrStmt>\
    static bool match(ExprOrStmt expr, const std::tuple<std::nullptr_t>&) {\
      return isa<TYPE>(expr);\
    }\
    \
    template <typename ExprOrStmt, typename Callback, typename Sub1>\
    static bool match(ExprOrStmt expr,\
                      const std::tuple<Callback, Sub1>& callbacks) {\
      if (isa<TYPE>(expr)) {\
        const TYPE* node = to<TYPE>(expr);\
        return HandleCallbackArguments::call(node, std::get<0>(callbacks)) &&\
               PatternMatch<T1>::match(node->MEMBER1, std::get<1>(callbacks));\
      }\
      return false;\
    }\
  };

#define GENERATE_PATTERNMATCH_2(TYPE, MEMBER1, MEMBER2)\
  template <typename T1, typename T2>\
  class PatternMatch<TYPE(T1, T2)> {\
  public:\
    template <typename ExprOrStmt>\
    static bool match(ExprOrStmt expr, std::nullptr_t) {\
      return isa<TYPE>(expr);\
    }\
    \
    template <typename ExprOrStmt>\
    static bool match(ExprOrStmt expr, const std::tuple<std::nullptr_t>&) {\
      return isa<TYPE>(expr);\
    }\
    \
    template <typename ExprOrStmt, typename Callback,\
              typename Sub1, typename Sub2>\
    static bool match(ExprOrStmt expr,\
                const std::tuple<Callback,Sub1, Sub2>& callbacks) {\
      if (isa<TYPE>(expr)) {\
        const TYPE* node = to<TYPE>(expr);\
        return HandleCallbackArguments::call(node, std::get<0>(callbacks)) &&\
               PatternMatch<T1>::match(node->MEMBER1, std::get<1>(callbacks)) &&\
               PatternMatch<T2>::match(node->MEMBER2, std::get<2>(callbacks));\
      }\
      return false;\
    }\
  };

#define GENERATE_PATTERNMATCH_3(TYPE, MEMBER1, MEMBER2, MEMBER3)\
  template <typename T1, typename T2, typename T3>\
  class PatternMatch<TYPE(T1, T2, T3)> {\
  public:\
    template <typename ExprOrStmt>\
    static bool match(ExprOrStmt expr, std::nullptr_t) {\
      return isa<TYPE>(expr);\
    }\
    \
    template <typename ExprOrStmt>\
    static bool match(ExprOrStmt expr, const std::tuple<std::nullptr_t>&) {\
      return isa<TYPE>(expr);\
    }\
    \
    template <typename ExprOrStmt, typename Callback,\
              typename Sub1, typename Sub2, typename Sub3>\
    static bool match(ExprOrStmt expr,\
                      const std::tuple<Callback, Sub1, Sub2, Sub3>& callbacks) {\
      if (isa<TYPE>(expr)) {\
        const TYPE* node = to<TYPE>(expr);\
        return HandleCallbackArguments::call(node, std::get<0>(callbacks)) &&\
               PatternMatch<T1>::match(node->MEMBER1, std::get<1>(callbacks)) &&\
               PatternMatch<T2>::match(node->MEMBER2, std::get<2>(callbacks)) &&\
               PatternMatch<T3>::match(node->MEMBER3, std::get<3>(callbacks));\
      }\
      return false;\
    }\
  };

GENERATE_PATTERNMATCH_0(Literal)
GENERATE_PATTERNMATCH_0(VarExpr)
GENERATE_PATTERNMATCH_2(Load, buffer, index)
GENERATE_PATTERNMATCH_1(FieldRead, elementOrSet)
GENERATE_PATTERNMATCH_0(Length)
GENERATE_PATTERNMATCH_1(IndexRead, edgeSet)

template <typename T1>
class PatternMatch<UnaryExpr*(T1)> {
public:
  template <typename ExprOrStmt>
  static bool match(ExprOrStmt expr, std::nullptr_t) {
    return isa<UnaryExpr>(expr);
  }

  template <typename ExprOrStmt>
  static bool match(ExprOrStmt expr, const std::tuple<std::nullptr_t>&) {
    return isa<UnaryExpr>(expr);
  }

  template <typename ExprOrStmt, typename Callback, typename Sub1>
  static bool match(ExprOrStmt expr,
                    const std::tuple<Callback, Sub1>& callbacks) {
    if (isa<UnaryExpr>(expr)) {
      const UnaryExpr* node = to<UnaryExpr>(expr);
      return HandleCallbackArguments::call(node, std::get<0>(callbacks)) &&
             PatternMatch<T1>::match(node->a, std::get<1>(callbacks));
    }
    return false;
  }
};

template <typename T1, typename T2>
class PatternMatch<BinaryExpr*(T1, T2)> {
public:
  template <typename ExprOrStmt>
  static bool match(ExprOrStmt expr, std::nullptr_t) {
    return isa<BinaryExpr>(expr);
  }

  template <typename ExprOrStmt>
  static bool match(ExprOrStmt expr, const std::tuple<std::nullptr_t>&) {
    return isa<BinaryExpr>(expr);
  }

  template <typename ExprOrStmt, typename Callback, typename Sub1, typename Sub2>
  static bool match(ExprOrStmt expr,
                    const std::tuple<Callback,Sub1, Sub2>& callbacks) {
    if (isa<BinaryExpr>(expr)) {
      const BinaryExpr* node = to<BinaryExpr>(expr);
      return HandleCallbackArguments::call(node, std::get<0>(callbacks)) &&
             PatternMatch<T1>::match(node->a, std::get<1>(callbacks)) &&
             PatternMatch<T2>::match(node->b, std::get<2>(callbacks));
    }
    return false;
  }
};

GENERATE_PATTERNMATCH_1(Neg, a)
GENERATE_PATTERNMATCH_2(Add, a, b)
GENERATE_PATTERNMATCH_2(Sub, a, b)
GENERATE_PATTERNMATCH_2(Mul, a, b)
GENERATE_PATTERNMATCH_2(Div, a, b)
GENERATE_PATTERNMATCH_2(Rem, a, b)

GENERATE_PATTERNMATCH_1(Not, a)
GENERATE_PATTERNMATCH_2(Eq, a, b)
GENERATE_PATTERNMATCH_2(Ne, a, b)
GENERATE_PATTERNMATCH_2(Gt, a, b)
GENERATE_PATTERNMATCH_2(Lt, a, b)
GENERATE_PATTERNMATCH_2(Ge, a, b)
GENERATE_PATTERNMATCH_2(Le, a, b)
GENERATE_PATTERNMATCH_2(And, a, b)
GENERATE_PATTERNMATCH_2(Or, a, b)
GENERATE_PATTERNMATCH_2(Xor, a, b)

GENERATE_PATTERNMATCH_0(VarDecl)
GENERATE_PATTERNMATCH_1(AssignStmt, value)
GENERATE_PATTERNMATCH_3(Store, buffer, index, value)
GENERATE_PATTERNMATCH_2(FieldWrite, elementOrSet, value)

// call

GENERATE_PATTERNMATCH_1(Scope, scopedStmt)
GENERATE_PATTERNMATCH_3(IfThenElse, condition, thenBody, elseBody)
GENERATE_PATTERNMATCH_3(ForRange, start, end, body)
GENERATE_PATTERNMATCH_1(For, body)
GENERATE_PATTERNMATCH_2(While, condition, body)
GENERATE_PATTERNMATCH_1(Kernel, body)

// block

GENERATE_PATTERNMATCH_1(Print, expr)
GENERATE_PATTERNMATCH_1(Comment,commentedStmt)
GENERATE_PATTERNMATCH_0(Pass)

GENERATE_PATTERNMATCH_2(UnnamedTupleRead, tuple, index)
GENERATE_PATTERNMATCH_1(NamedTupleRead, tuple)
// setread
// tensorread
//GENERATE_PATTERNMATCH_(TensorWrite)
GENERATE_PATTERNMATCH_1(IndexedTensor, tensor)
GENERATE_PATTERNMATCH_1(IndexExpr, value)
//GENERATE_PATTERNMATCH_(Map)

}
}

#endif

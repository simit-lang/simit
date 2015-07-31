#ifndef SIMIT_VARIADIC_H
#define SIMIT_VARIADIC_H

#include <type_traits>

namespace simit {
namespace util {


// Compare types
template<class T, class...>
struct areSame : std::true_type {};

template<class T1, class T2, class... TN>
struct areSame<T1, T2, TN...>
  : std::integral_constant<bool, std::is_same<T1,T2>{} && areSame<T1, TN...>{}>
{};


// Product
namespace {

template <int...>
struct productHelper;

template <int prod, int val, int... rest>
struct productHelper<prod, val, rest...> {
  static const int value = productHelper<prod * val, rest...>::value;
};


template <int val>
struct productHelper<val> {
  static const int value = val;
};

} // unnamed namespace

template <int... vals> struct product;

template <>
struct product <> {
  static const int value = 1;
};

template <int val, int... rest>
struct product <val, rest...> {
  static const int value = productHelper<1, val, rest...>::value;
};

template <int... vals>
struct product {
  static const int value = product<vals...>::value;
};


// Machinery to compute array offsets
template <int...> struct seq {};

/// Remove first value from int variadic template
template <int first, int... rest>
struct removeFirst {
  typedef seq<rest...> type;
};

/// Compute product of static sequence
template <int... values>
inline int computeProduct(seq<values...> seq) {
  return product<values...>::value;
}

template <typename... Indices, int... dimensions>
inline int computeOffset(Indices... indices, const seq<dimensions...> &dims);

template <int... dimensions>
inline int computeOffset(const seq<dimensions...> &dims, int i) {
  return i;
}

/// Compute the offset into an n-dimensional array
template <int... dimensions, typename... Indices>
inline int computeOffset(seq<dimensions...> dims, int index, Indices... rest) {
  typename removeFirst<dimensions...>::type innerDims;
  return index * computeProduct(innerDims) + computeOffset(innerDims, rest...);
}

}} // namespace simit::util

#endif

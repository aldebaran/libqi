#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_HASLESS_HXX_
#define _QITYPE_DETAIL_HASLESS_HXX_

#include <qi/type/typeinterface.hpp>
#include <ka/macroregular.hpp>
#include <ka/typetraits.hpp>
#include <boost/optional.hpp>
#include <boost/type_traits/has_less.hpp>
#include <map>
#include <set>
#include <tuple>

namespace qi
{
namespace detail
{

  // Some types are not handled by boost::has_less. This trait enables us to detect them and process
  // them differently.
  template <typename T>
  using CanBeUsedInHasLess =
    ka::Negation<
      ka::Disjunction<
        // Functions and function pointers are not handled.
        std::is_function<ka::RemovePointer<T>>,
        // Member function pointers are considered as member pointers. Neither member function
        // pointers nor member object pointers are handled.
        std::is_member_pointer<T>
      >
    >;

  template <typename T>
  struct HasLess : ka::Conjunction<CanBeUsedInHasLess<T>, boost::has_less<T, T, bool>> {};

  // The following types for which we define a specialisation of HasLess are types supported by
  // the qi type system for which the less operator depends on the template parameters.
  // STL containers use a lexicographical compare of their elements which have the type of the
  // template parameter.
  template<typename T, typename Alloc>
  struct HasLess<std::vector<T, Alloc>> : HasLess<T> {};

  template<typename T, typename Alloc>
  struct HasLess<std::list<T, Alloc>> : HasLess<T> {};

  // The `less` operator of `std::set` does not use the `Compare` type but instead does a
  // lexicographical comparison of their elements. Therefore having `less` on `std::set` requires
  // having `less` on the element type.
  template<typename T, typename Compare, typename Alloc>
  struct HasLess<std::set<T, Compare, Alloc>> : HasLess<T> {};

  // The `less` operator of `std::map` does not use the `Compare` type but instead does a
  // lexicographical comparison of their elements. Therefore having `less` on `std::map` requires
  // having `less` on the element type (which is a `pair<const K, V>`, so to avoid confusion, we
  // just refer to the value_type typedef).
  template <typename K, typename V, typename Compare, typename Alloc>
  struct HasLess<std::map<K, V, Compare, Alloc>>
    : HasLess<typename std::map<K, V, Compare, Alloc>::value_type> {};

  template<typename A, typename B>
  struct HasLess<std::pair<A, B>> : ka::Conjunction<HasLess<A>, HasLess<B>> {};

  template<typename... T>
  struct HasLess<std::tuple<T...>> : ka::Conjunction<HasLess<T>...> {};

  template<typename T>
  struct HasLess<boost::optional<T>> : HasLess<T> {};

#if QI_HAS_STD_OPTIONAL
  template<typename T>
  struct HasLess<std::optional<T>> : HasLess<T> {};
#endif

  template<typename T>
  struct LessDeref
  {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(LessDeref)

  // Relation:
    bool operator()(const T& a, const T& b) const
    {
      return *a < *b;
    }
  };

  /// model Procedure<bool(T*, T*)> Less<T>:
  /// If T is LessComparable, then the pointers are dereferenced and the results are compared.
  /// Otherwise, the value of the pointers are compared.
  template<typename T>
  using Less = ka::Conditional<HasLess<T>::value, LessDeref<T*>, std::less<T*>>;

} // namespace detail
} // namespace qi

#endif  // _QITYPE_DETAIL_HASLESS_HXX_

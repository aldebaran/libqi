#pragma once
/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_TRAITS_HPP_
#define _QI_TYPE_TRAITS_HPP_

#include <type_traits>
#include <qi/macro.hpp>
#include <boost/container/container_fwd.hpp>

// Unfortunately, the standard does not allow to add anything to the
// std namespace, except explicit specializations of existing functions.
// That implies that it is forbidden to open the std namespace to forward
// declare std types (like containers).
// So here we do inclusions instead of forward declarations.
#include <string>
#include <deque>
#include <vector>
#include <array>
#include <forward_list>
#include <list>

namespace qi
{
  /// Contains anything related to compile-time type manipulation.
  namespace traits
  {
    using True  = std::true_type;
    using False = std::false_type;

    template<typename A, typename B>
    using Equal = typename std::is_same<A, B>::type;

    namespace detail
    {
      template<typename A>
      struct RemoveCvImpl : std::remove_cv<A>
      {
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveCvImpl<Ret (C::*)(Arg...) const>
      {
        using type = Ret (C::*)(Arg...);
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveCvImpl<Ret (C::*)(Arg...) volatile>
      {
        using type = Ret (C::*)(Arg...);
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveCvImpl<Ret (C::*)(Arg...) const volatile>
      {
        using type = Ret (C::*)(Arg...);
      };
    } // namespace detail

    /// Remove const and volatile on all types, including member functions.
    /// std::remove_cv doesn't handle the latter case.
    template<typename A>
    using RemoveCv = typename detail::RemoveCvImpl<A>::type;

    namespace detail
    {
      template<typename T>
      struct RemoveRefImpl : std::remove_reference<T>
      {
      };

#if QI_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
      template<typename Ret, typename C, typename... Arg>
      struct RemoveRefImpl<Ret (C::*)(Arg...) &>
      {
        using type = Ret (C::*)(Arg...);
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveRefImpl<Ret (C::*)(Arg...) const &>
      {
        using type = Ret (C::*)(Arg...) const;
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveRefImpl<Ret (C::*)(Arg...) volatile &>
      {
        using type = Ret (C::*)(Arg...) volatile;
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveRefImpl<Ret (C::*)(Arg...) const volatile &>
      {
        using type = Ret (C::*)(Arg...) const volatile;
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveRefImpl<Ret (C::*)(Arg...) &&>
      {
        using type = Ret (C::*)(Arg...);
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveRefImpl<Ret (C::*)(Arg...) const &&>
      {
        using type = Ret (C::*)(Arg...) const;
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveRefImpl<Ret (C::*)(Arg...) volatile &&>
      {
        using type = Ret (C::*)(Arg...) volatile;
      };

      template<typename Ret, typename C, typename... Arg>
      struct RemoveRefImpl<Ret (C::*)(Arg...) const volatile &&>
      {
        using type = Ret (C::*)(Arg...) const volatile;
      };
#endif
    } // namespace detail

    /// Remove reference (l-value reference and r-value reference, including on member functions).
    template<typename A>
    using RemoveRef = typename detail::RemoveRefImpl<A>::type;

    /// Remove the reference, then the const / volatile qualifier.
    /// For example, for the type A const&, this will return A.
    template<typename A>
    using RemoveCvRef = RemoveCv<RemoveRef<A>>;

    namespace detail
    {
      template<typename A>
      struct HasMemberOperatorCallImpl
      {
        template<typename B>
        static True test(decltype(&B::operator(), 0));

        template<typename>
        static False test(...);

        using type = decltype(test<A>(0));
      };
    } // namespace detail

    template<typename A>
    using HasMemberOperatorCall = typename detail::HasMemberOperatorCallImpl<A>::type;

    namespace detail
    {
      template<typename A>
      struct IsFunctionObjectImpl : HasMemberOperatorCall<A>
      {
      };
    }

    /// A FunctionObject is an object that can be used on the left of the function call operator.
    /// This is different from Callable, which includes builtin functions.
    template<typename A>
    using IsFunctionObject = typename detail::IsFunctionObjectImpl<A>::type;

    namespace detail
    {
      template<typename A>
      struct FunctionImplFunctionObject;

      template<typename Ret, typename C, typename... Arg>
      struct FunctionImplFunctionObject<Ret (C::*)(Arg...)>
      {
        // A typedef is used here instead of a using because of a VS 2013 bug.
        // TODO: Replace it by a using when moving to VS 2015.
        typedef Ret (type)(Arg...);
      };

      template<typename A>
      struct FunctionImpl : FunctionImplFunctionObject<RemoveCv<decltype(&A::operator())>>
      {
      };

      template<typename Ret, typename... Arg>
      struct FunctionImpl<Ret (Arg...)>
      {
        // A typedef is used here instead of a using because of a VS 2013 bug.
        // TODO: Replace it by a using when moving to VS 2015.
        typedef Ret (type)(Arg...);
      };

      template<typename Ret, typename... Arg>
      struct FunctionImpl<Ret (*)(Arg...)>
      {
        // A typedef is used here instead of a using because of a VS 2013 bug.
        // TODO: Replace it by a using when moving to VS 2015.
        typedef Ret (type)(Arg...);
      };
    } // namespace detail

    /// Returns the function type for the passed callable.
    /// Callable -> BuiltinFunction
    /// For example, the type char const* (float&, int) is returned for decltype([](float&, int) {return "abc";})
    template<typename A>
    using Function = typename detail::FunctionImpl<A>::type;

    namespace detail
    {
      // Below are definitions and specializations of IsContiguous and IsList
      // for standard containers and the most common boost containers.
      // If a type is not specialized here, you can anywhere reopen this
      // namespace and add your specialization.
      // These traits can be used to implement algorithm optimizations
      // (see qi::erase_if for example).

      /// Contiguous means that the type is associated to elements that are
      /// placed at adjacent memory addresses, without any hole.
      /// By default, a type is not contiguous.
      /// Specializations are use to indicate types that _are_ contiguous.
      template<typename T>
      struct IsContiguous
      {
        using type = False;
      };

      template<typename T, std::size_t N>
      struct IsContiguous<std::array<T, N>>
      {
        using type = True;
      };

      template<typename T, typename A>
      struct IsContiguous<std::vector<T, A>>
      {
        using type = True;
      };

      template<typename C, typename T, typename A>
      struct IsContiguous<std::basic_string<C, T, A>>
      {
        using type = True;
      };

      template<typename T, std::size_t N>
      struct IsContiguous<boost::container::static_vector<T, N>>
      {
        using type = True;
      };

      template<typename T, std::size_t N, typename A>
      struct IsContiguous<boost::container::small_vector<T, N, A>>
      {
        using type = True;
      };

      template<typename K, typename C, typename A>
      struct IsContiguous<boost::container::flat_set<K, C, A>>
      {
        using type = True;
      };

      template<typename K, typename C, typename A>
      struct IsContiguous<boost::container::flat_multiset<K, C, A>>
      {
        using type = True;
      };

      template<typename K, typename V, typename C, typename A>
      struct IsContiguous<boost::container::flat_map<K, V, C, A>>
      {
        using type = True;
      };

      template<typename K, typename V, typename C, typename A>
      struct IsContiguous<boost::container::flat_multimap<K, V, C, A>>
      {
        using type = True;
      };

      /// A type is "contiguous like" if its associated elements are placed at
      /// adjacent memory addresses, in one or several chunks, as long as it's
      /// possible to navigate between them in a random access manner.
      /// By default, a type is "contiguous like" if it is contiguous.
      /// Specializations are use to indicate types that _are_ "contiguous like".
      /// See qi::IsContiguous for more information.
      template<typename T>
      struct IsContiguousLike : IsContiguous<T>
      {
      };

      /// A deque is "contiguous like" because, even if it maintains potentially
      /// several chunks of memory, its iterators are random access.
      template<typename T, typename A>
      struct IsContiguousLike<std::deque<T, A>>
      {
        using type = True;
      };

      /// A type is a list if its associated elements are some kind of nodes and
      /// the navigation between them is not random access.
      /// By default, a type is not a list.
      /// Specializations are use to indicate types that _are_ lists.
      template<typename T>
      struct IsList
      {
        using type = False;
      };

      template<typename T, typename A>
      struct IsList<std::forward_list<T, A>>
      {
        using type = True;
      };

      template<typename T, typename A>
      struct IsList<std::list<T, A>>
      {
        using type = True;
      };

      template<typename T, typename A>
      struct IsList<boost::container::slist<T, A>>
      {
        using type = True;
      };
    }

    /// True if the underlying memory is contiguous.
    /// Note that this trait is not about Container or Sequence : std::array and
    /// std::basic_string are Contiguous even if they are not Container or Sequence.
    /// From C++17 draft:
    /// 23.2.1/13 General container requirements [container.requirements.general]
    template<typename T>
    using IsContiguous = typename detail::IsContiguous<T>::type;

    /// True if the underlying memory is contiguous, or "almost" as in std::deque.
    template<typename T>
    using IsContiguousLike = typename detail::IsContiguousLike<T>::type;

    /// True if the memory is sequentially arranged in nodes.
    /// Example: std::forward_list, std::list
    template<typename T>
    using IsList = typename detail::IsList<T>::type;

  } // namespace traits
} // namespace qi

#endif  // _QITYPE_TYPETRAITS_HPP_

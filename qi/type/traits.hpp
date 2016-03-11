#pragma once
/*
**  Copyright (C) 2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_TRAITS_HPP_
#define _QI_TYPE_TRAITS_HPP_

#include <type_traits>

namespace qi
{
  /// Contains anything related to compile-time type manipulation.
  namespace type
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

    /// Remove reference (normal reference and r-value reference).
    template<typename A>
    using RemoveRef = typename std::remove_reference<A>::type;

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

  } // namespace type
} // namespace qi

#endif  // _QITYPE_TYPETRAITS_HPP_

#ifndef KA_TYPETRAITS_HPP
#define KA_TYPETRAITS_HPP
#pragma once
#include <iterator>
#include <type_traits>
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

#include "utility.hpp"
#include "macro.hpp"

namespace ka {
  using true_t  = std::true_type;
  using false_t = std::false_type;

  template<typename A, typename B>
  using Equal = typename std::is_same<A, B>::type;

  namespace detail {
    template<typename A>
    struct RemoveCvImpl : std::remove_cv<A> {
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveCvImpl<Ret (C::*)(Arg...) const> {
      using type = Ret (C::*)(Arg...);
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveCvImpl<Ret (C::*)(Arg...) volatile> {
      using type = Ret (C::*)(Arg...);
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveCvImpl<Ret (C::*)(Arg...) const volatile> {
      using type = Ret (C::*)(Arg...);
    };
  } // namespace detail

  /// Remove const and volatile on all types, including member functions.
  /// std::remove_cv doesn't handle the latter case.
  template<typename A>
  using RemoveCv = typename detail::RemoveCvImpl<A>::type;

  namespace detail {
    template<typename T>
    struct RemoveRefImpl : std::remove_reference<T> {
    };

#if KA_COMPILER_SUPPORTS_MEMBER_FUNCTION_REF_QUALIFIERS
    template<typename Ret, typename C, typename... Arg>
    struct RemoveRefImpl<Ret (C::*)(Arg...) &> {
      using type = Ret (C::*)(Arg...);
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveRefImpl<Ret (C::*)(Arg...) const &> {
      using type = Ret (C::*)(Arg...) const;
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveRefImpl<Ret (C::*)(Arg...) volatile &> {
      using type = Ret (C::*)(Arg...) volatile;
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveRefImpl<Ret (C::*)(Arg...) const volatile &> {
      using type = Ret (C::*)(Arg...) const volatile;
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveRefImpl<Ret (C::*)(Arg...) &&> {
      using type = Ret (C::*)(Arg...);
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveRefImpl<Ret (C::*)(Arg...) const &&> {
      using type = Ret (C::*)(Arg...) const;
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveRefImpl<Ret (C::*)(Arg...) volatile &&> {
      using type = Ret (C::*)(Arg...) volatile;
    };

    template<typename Ret, typename C, typename... Arg>
    struct RemoveRefImpl<Ret (C::*)(Arg...) const volatile &&> {
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

  /// Generates a trait evaluating to `std::true_type` iff the given expression
  /// is valid.
  ///
  /// The generated trait is a template alias, the underlying trait being in the
  /// `detail` namespace.
  ///
  /// Also, because of the use of `std::true_type` and `std::false_type`,
  /// the standard header `<type_traits>` must already be included.
  ///
  /// Example: Generating a trait testing (for a type parameter `T`) the validity of `*t`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// #include <type_traits>
  ///
  /// KA_GENERATE_TRAITS_HAS(HasOperatorStar, T, *std::declval<T>())
  /// static_assert( HasOperatorStar<int*>::value, "");
  /// static_assert( HasOperatorStar<std::vector<int>::iterator>::value, "");
  /// static_assert(!HasOperatorStar<int>::value, "");
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Note: Useful functions to "produce" value in the expression are:
  /// `std::declval`, `declref` and `declcref` (see utility.hpp).
  #define KA_GENERATE_TRAITS_HAS(TRAIT_NAME, TYPE_PARAM, EXPR) \
    namespace detail {                                         \
      template<typename TYPE_PARAM##_>                         \
      struct TRAIT_NAME {                                      \
        template<typename TYPE_PARAM>                          \
        static std::true_type test(decltype((EXPR), 0));       \
                                                               \
        template<typename>                                     \
        static std::false_type test(...);                      \
                                                               \
        using type = decltype(test<TYPE_PARAM##_>(0));         \
      };                                                       \
    }                                                          \
    template<typename T>                                       \
    using TRAIT_NAME = typename detail::TRAIT_NAME<T>::type;

  KA_GENERATE_TRAITS_HAS(HasMemberOperatorCall, T, &T::operator())

  namespace detail {
    template<typename A>
    struct IsFunctionObjectImpl : HasMemberOperatorCall<A> {
    };
  }

  /// A FunctionObject is an object that can be used on the left of the function call operator.
  /// This is different from Callable, which includes builtin functions.
  template<typename A>
  using IsFunctionObject = typename detail::IsFunctionObjectImpl<A>::type;

  namespace detail {
    template<typename A>
    struct FunctionImplFunctionObject;

    template<typename Ret, typename C, typename... Arg>
    struct FunctionImplFunctionObject<Ret (C::*)(Arg...)> {
      // A typedef is used here instead of a using because of a VS 2013 bug.
      // TODO: Replace it by a using when moving to VS 2015.
      typedef Ret (type)(Arg...);
    };

    template<typename A>
    struct FunctionImpl : FunctionImplFunctionObject<RemoveCv<decltype(&A::operator())>> {
    };

    template<typename Ret, typename... Arg>
    struct FunctionImpl<Ret (Arg...)> {
      // A typedef is used here instead of a using because of a VS 2013 bug.
      // TODO: Replace it by a using when moving to VS 2015.
      typedef Ret (type)(Arg...);
    };

    template<typename Ret, typename... Arg>
    struct FunctionImpl<Ret (*)(Arg...)> {
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

  namespace detail {
    // Below are definitions and specializations of IsContiguous and IsList
    // for standard containers and the most common boost containers.
    // If a type is not specialized here, you can anywhere reopen this
    // namespace and add your specialization.
    // These traits can be used to implement algorithm optimizations
    // (see ka::erase_if for example).

    /// Contiguous means that the type is associated to elements that are
    /// placed at adjacent memory addresses, without any hole.
    /// By default, a type is not contiguous.
    /// Specializations are use to indicate types that _are_ contiguous.
    template<typename T>
    struct IsContiguous {
      using type = false_t;
    };

    template<typename T, std::size_t N>
    struct IsContiguous<std::array<T, N>> {
      using type = true_t;
    };

    template<typename T, typename A>
    struct IsContiguous<std::vector<T, A>> {
      using type = true_t;
    };

    template<typename C, typename T, typename A>
    struct IsContiguous<std::basic_string<C, T, A>> {
      using type = true_t;
    };

    template<typename T, std::size_t N>
    struct IsContiguous<boost::container::static_vector<T, N>> {
      using type = true_t;
    };

    template<typename T, std::size_t N, typename A>
    struct IsContiguous<boost::container::small_vector<T, N, A>> {
      using type = true_t;
    };

    template<typename K, typename C, typename A>
    struct IsContiguous<boost::container::flat_set<K, C, A>> {
      using type = true_t;
    };

    template<typename K, typename C, typename A>
    struct IsContiguous<boost::container::flat_multiset<K, C, A>> {
      using type = true_t;
    };

    template<typename K, typename V, typename C, typename A>
    struct IsContiguous<boost::container::flat_map<K, V, C, A>> {
      using type = true_t;
    };

    template<typename K, typename V, typename C, typename A>
    struct IsContiguous<boost::container::flat_multimap<K, V, C, A>> {
      using type = true_t;
    };

    /// A type is "contiguous like" if its associated elements are placed at
    /// adjacent memory addresses, in one or several chunks, as long as it's
    /// possible to navigate between them in a random access manner.
    /// By default, a type is "contiguous like" if it is contiguous.
    /// Specializations are use to indicate types that _are_ "contiguous like".
    /// See IsContiguous for more information.
    template<typename T>
    struct IsContiguousLike : IsContiguous<T> {
    };

    /// A deque is "contiguous like" because, even if it maintains potentially
    /// several chunks of memory, its iterators are random access.
    template<typename T, typename A>
    struct IsContiguousLike<std::deque<T, A>> {
      using type = true_t;
    };

    /// A type is a list if its associated elements are some kind of nodes and
    /// the navigation between them is not random access.
    /// By default, a type is not a list.
    /// Specializations are use to indicate types that _are_ lists.
    template<typename T>
    struct IsList {
      using type = false_t;
    };

    template<typename T, typename A>
    struct IsList<std::forward_list<T, A>> {
      using type = true_t;
    };

    template<typename T, typename A>
    struct IsList<std::list<T, A>> {
      using type = true_t;
    };

    template<typename T, typename A>
    struct IsList<boost::container::slist<T, A>> {
      using type = true_t;
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

  /// Behave exactly as the std::decay_t of C++14.
  template<typename T>
  using Decay = typename std::decay<T>::type;

  /// Behave exactly as the std::enable_if_t of C++14.
  template<bool condition, typename T = void>
  using EnableIf = typename std::enable_if<condition, T>::type;

  /// Behave exactly as the std::underlying_type_t of C++14.
  template<typename T>
  using UnderlyingType = typename std::underlying_type<T>::type;

  /// Useful to avoid a template constructor to swallow copy constructor,
  /// move constructor, etc.
  ///
  /// Note: The "derived" type is decayed as it corresponds to the most common case.
  ///
  /// Example: leaving untouched copy constructor and assignment operator
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // will not swallow the copy constructor
  /// template<typename U, typename = EnableIfNotBaseOf<MutableStore, U>>
  /// explicit MutableStore(U&& u)
  ///   : data(std::forward<U>(u)) {
  /// }
  ///
  /// template<typename U, typename = EnableIfNotBaseOf<MutableStore, U>>
  /// MutableStore& operator=(U&& u) {
  ///   data = std::forward<U>(u);
  ///   return *this;
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template<typename Base, typename Derived>
  using EnableIfNotBaseOf = EnableIf<! std::is_base_of<Base, Decay<Derived>>::value>;

  /// Behave exactly as the std::result_of_t of C++14.
  template<typename T>
  using ResultOf = typename std::result_of<T>::type;

  /// Behave exactly as the std::remove_pointer_t of C++14.
  template<typename T>
  using RemovePointer = typename std::remove_pointer<T>::type;

  /// Behave exactly as the std::conditional_t of C++14.
  template<bool B, typename T, typename F>
  using Conditional = typename std::conditional<B, T, F>::type;

  /// Behave exactly as the std::conjunction of C++17
  template<typename...> struct Conjunction : true_t {};
  template<typename B1> struct Conjunction<B1> : B1 {};
  template<typename B1, typename... Bn>
  struct Conjunction<B1, Bn...>
    : Conditional<bool(B1::value), Conjunction<Bn...>, B1> {};

  KA_GENERATE_TRAITS_HAS(HasOperatorStar, T, *std::declval<T>())


// TODO: Removes this workaround when get rid of VS2013.
#if KA_COMPILER_VS2013_OR_BELOW
  namespace detail {
    template<> struct HasOperatorStar<bool> : false_t {};
    template<> struct HasOperatorStar<char> : false_t {};
    template<> struct HasOperatorStar<short> : false_t {};
    template<> struct HasOperatorStar<int> : false_t {};
    template<> struct HasOperatorStar<long> : false_t {};
    template<> struct HasOperatorStar<long long> : false_t {};
    template<> struct HasOperatorStar<float> : false_t {};
    template<> struct HasOperatorStar<double> : false_t {};
    template<> struct HasOperatorStar<long double> : false_t {};
    template<> struct HasOperatorStar<unsigned char> : false_t {};
    template<> struct HasOperatorStar<unsigned short> : false_t {};
    template<> struct HasOperatorStar<unsigned int> : false_t {};
    template<> struct HasOperatorStar<unsigned long> : false_t {};
    template<> struct HasOperatorStar<unsigned long long> : false_t {};
  } // namespace detail

  /// Returns the retraction of a (retractable) function.
  ///
  /// See the concept `RetractableFunction` in concept.hpp
  ///
  /// RetractableFunction F
  template<typename F>
  using Retract = Decay<typename F::retract_type>;

  KA_GENERATE_TRAITS_HAS(HasRetract, T, typename T::retract_type{})
#else
  /// Returns the retraction of a (retractable) function.
  ///
  /// See the concept `RetractableFunction` in concept.hpp
  ///
  /// RetractableFunction F
  template<typename F>
  using Retract = Decay<decltype(retract(declcref<F>()))>;

  KA_GENERATE_TRAITS_HAS(HasRetract, T, retract(declcref<T>()))
#endif

  namespace detail {
    template<typename G, typename F, typename FHasRetract>
    struct IsRetract : std::is_same<Retract<F>, G> {
    };

    template<typename G, typename F>
    struct IsRetract<G, F, false_t> : false_t {
    };
  } // namespace detail

  /// True if `G` is a retraction of `F`.
  ///
  /// More precisely, the inner `type` is `true_t` iff `G` is a
  /// retraction of `F`.
  ///
  /// The algorithm is:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// if ((F has a `retract` free function) && (this `retract`'s return type is `G`))
  ///   derive from `true_t`
  /// else
  ///   derive from `false_t`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// See the concept `RetractableFunction` in concept.hpp
  ///
  /// Function G, Function F
  template<typename G, typename F>
  struct IsRetract : detail::IsRetract<G, F, HasRetract<Decay<F>>> {
  };

  KA_GENERATE_TRAITS_HAS(HasInputIteratorTag, T,
    std::declval<std::input_iterator_tag>() = typename std::iterator_traits<T>::iterator_category{})

  /// Causes a substitution failure if the type is not an iterator, effectively
  /// discarding the function from the overload set.
  ///
  /// Note: `EnableIfInputIterator` is to be typically used in conjunction with
  ///       `EnableIfNotInputIterator`.
  ///
  /// Example: Making an overload visible only for iterators, and another one
  ///          for other types
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// /// InputIterator I
  /// template<typename I, typename = ka::EnableIfInputIterator<I>>
  /// I distance(I b, I e);
  ///
  /// /// Arithmetic N
  /// template<typename N, typename = ka::EnableIfNotInputIterator<N>>
  /// N distance(N b, N e);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template<typename T>
  using EnableIfInputIterator = EnableIf<HasInputIteratorTag<T>::value>;

  /// Causes a substitution failure if the type is an iterator, effectively
  /// discarding the function from the overload set.
  ///
  /// See `EnableIfInputIterator` for a usage example.
  template<typename T>
  using EnableIfNotInputIterator = EnableIf<!HasInputIteratorTag<T>::value>;
} // namespace ka

#endif // KA_TYPETRAITS_HPP

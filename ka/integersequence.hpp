#ifndef KA_TYPE_INTEGERSEQUENCE_HPP
#define KA_TYPE_INTEGERSEQUENCE_HPP
#pragma once
#include <boost/config.hpp>

/// @file Implements std::integer_sequence types from C++14.
///
/// This implementation is from the stdlib shipped with g++7.
///
/// TODO: Remove this file when C++14 is available.

namespace ka
{
  namespace detail
  {
    // Stores a tuple of indices. Used by tuple and pair, and by bind() to
    // extract the elements in a tuple.
    template<size_t... Indexes>
    struct Index_tuple
    {
    };

    // Concatenates two Index_tuples.
    template<typename Itup1, typename Itup2>
    struct Itup_cat;

    template<size_t... Ind1, size_t... Ind2>
    struct Itup_cat<Index_tuple<Ind1...>, Index_tuple<Ind2...>>
    {
      using type = Index_tuple<Ind1..., (Ind2 + sizeof...(Ind1))...>;
    };

    // Builds an Index_tuple<0, 1, 2, ..., Num-1>.
    template<size_t Num>
    struct Build_index_tuple
      : Itup_cat<typename Build_index_tuple<Num / 2>::type,
                 typename Build_index_tuple<Num - Num / 2>::type>
    {
    };

    template<>
    struct Build_index_tuple<1>
    {
      typedef Index_tuple<0> type;
    };

    template<>
    struct Build_index_tuple<0>
    {
      typedef Index_tuple<> type;
    };

  } // namespace detail

  /// Class template integer_sequence
  template<typename Tp, Tp... Idx>
  struct integer_sequence
  {
    typedef Tp value_type;
    static BOOST_CONSTEXPR size_t size()
    {
      return sizeof...(Idx);
    }
  };

  namespace detail
  {
    template<typename Tp, Tp Num, typename ISeq = typename Build_index_tuple<Num>::type>
    struct Make_integer_sequence;

    template<typename Tp, Tp Num,  size_t... Idx>
    struct Make_integer_sequence<Tp, Num, Index_tuple<Idx...>>
    {
      static_assert(Num >= 0,
         "Cannot make integer sequence of negative length");

      typedef integer_sequence<Tp, Idx...> type;
    };

    template<typename Tp, Tp Num>
    struct make_integer_sequence : detail::Make_integer_sequence<Tp, Num> {};
  } // namespace detail

  /// Alias template make_integer_sequence
  template<typename Tp, Tp Num>
  using make_integer_sequence = typename detail::make_integer_sequence<Tp, Num>::type;

  /// Alias template index_sequence
  template<size_t... Idx>
  using index_sequence = integer_sequence<size_t, Idx...>;

  /// Alias template make_index_sequence
  template<size_t Num>
  using make_index_sequence = typename detail::make_integer_sequence<size_t, Num>::type;

  /// Alias template index_sequence_for
  template<typename... Types>
  using index_sequence_for = make_index_sequence<sizeof...(Types)>;
} // namespace ka

#endif // KA_TYPE_INTEGERSEQUENCE_HPP

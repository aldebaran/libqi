#ifndef KA_ARK_INPUTITER_HPP
#define KA_ARK_INPUTITER_HPP
#pragma once
#include <iterator>
#include "../typetraits.hpp"
#include "../macro.hpp"
#include "../macroregular.hpp"

namespace ka {
namespace ark {

  namespace detail {
    template<typename T>
    T* operator_arrow(T* x) {
      return x;
    }

    template<typename T>
    T const* operator_arrow(T const* x) {
      return x;
    }

    template<typename T>
    auto operator_arrow(T const& x) -> decltype(x.operator->()) {
      return x.operator->();
    }
  } // namespace detail

  /// Archetype of a StdInputIterator.
  ///
  /// It can be seen as a interface restriction.
  ///
  /// Example: ensuring that an algorithm accepts a bare input iterator
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `v` is a `std::vector` so `v.begin()` is a random access iterator.
  /// // `my_algo` is supposed to accept an input iterator, so we enforce it
  /// // for test purpose.
  /// my_algo(input_iter(v.begin()), input_iter(v.end()));
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// StdInputIterator I
  template<typename I>
  struct input_iter_t {
    I i;

  // StdInputIterator:
    using iterator_category = std::input_iterator_tag;
    using value_type = typename std::iterator_traits<I>::value_type;
    using difference_type = typename std::iterator_traits<I>::difference_type;
    using pointer = typename std::iterator_traits<I>::pointer;
    using reference = typename std::iterator_traits<I>::reference;

    friend KA_GENERATE_REGULAR_OP_EQUAL_1(input_iter_t, i)
    friend KA_GENERATE_REGULAR_OP_DIFFERENT(input_iter_t)

    reference operator*() const {
      return *i;
    }

    pointer operator->() const {
      // It is unfortunately not possible to directly use `operator->` on `i`,
      // because `i` could be a raw pointer...
      // We resort to specializations to handle this.
      return detail::operator_arrow(i);
    }

    input_iter_t& operator++() {
      ++i;
      return *this;
    }

    auto operator++(int) -> decltype(i++) {
      return i++;
    }
  };

KA_DERIVE_CTOR_FUNCTION_TEMPLATE(input_iter)

}} // ka::ark

#endif // KA_ARK_INPUTITER_HPP

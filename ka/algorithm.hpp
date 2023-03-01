#ifndef KA_ALGORITHM_HPP
#define KA_ALGORITHM_HPP
#pragma once
#include <algorithm>
#include "typetraits.hpp"

namespace ka {

// TODO: When C++17 is available, check if this code is still needed and remove
// it if it's not.
//
// erase_if algorithms are coming in C++17 so these implementations strive to
// be compatible.
// See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/n4600.html#container.erasure.erase_if

namespace detail {
  /// See the comment of erase_if below for explanation on "contiguous like".
  ///
  /// ContiguousLikeSequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_contiguous_like(S& s, P p) {
    s.erase(std::remove_if(s.begin(), s.end(), p), s.end());
  }

  /// ListSequence<T> S, Predicate<T> P
  ///
  /// ListSequence is not a standard concept: it is a Sequence that behaves
  /// like std::list or std::forward_list, with a remove_if member function.
  template<typename S, typename P>
  void erase_if_list(S& s, P p) {
    s.remove_if(p);
  }

  /// Sequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_default(S& s, P p) {
    auto b = s.begin();
    auto const e = s.end();
    while (true) {
      b = std::find_if(b, e, p);
      if (b == e) return; // Too bad end is not erasable as a no-op...
      b = s.erase(b);
    }
  }

  /// ContiguousLikeSequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_dispatch(S& s, P const& p, true_t /*is_contiguous_like*/, false_t /*is_list*/) {
    erase_if_contiguous_like(s, p);
  }

  /// ListSequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_dispatch(S& s, P const& p, false_t /*is_contiguous_like*/, true_t /*is_list*/) {
    erase_if_list(s, p);
  }

  /// Sequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_dispatch(S& s, P const& p, false_t /*is_contiguous_like*/, false_t /*is_list*/) {
    erase_if_default(s, p);
  }
} // namespace detail

/// Erase all elements that respect the predicate.
///
/// Optimizations are performed for contiguous-like sequences (vector, deque...)
/// and for list sequences (list, forward_list).
/// If you want to take advantage on this for your own containers, specialize
/// IsContiguousLike or IsList.
/// Note: It would be nice to return the predicate, because it could be stateful
/// and for example count the number of true elements, but it's not possible because
/// underlying stl algorithms and member functions do not return the predicate...
///
/// Sequence<T> S, Predicate<T> P
template<typename S, typename P>
void erase_if(S& s, P const& p) {
  detail::erase_if_dispatch(s, p, IsContiguousLike<S>{}, IsList<S>{});
}

} // namespace ka

#endif  // KA_ALGORITHM_HPP

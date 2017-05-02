#pragma once
#ifndef _QI_ALGORITHM_HPP_
#define _QI_ALGORITHM_HPP_
#include <algorithm>
#include <qi/type/traits.hpp>

namespace qi
{

// erase_if algorithms are coming in C++17 so these implementations strive to
// be compatible.
// See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/n4600.html#container.erasure.erase_if

namespace detail
{
  /// See the comment of erase_if below for explanation on "contiguous like".
  /// ContiguousLikeSequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_contiguous_like(S& s, P p)
  {
    s.erase(std::remove_if(s.begin(), s.end(), p), s.end());
  }

  /// ListSequence<T> S, Predicate<T> P
  /// ListSequence is not a standard concept: it is a Sequence that behaves
  /// like std::list or std::forward_list, with a remove_if member function.
  template<typename S, typename P>
  void erase_if_list(S& s, P p)
  {
    s.remove_if(p);
  }

  /// Sequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_default(S& s, P p)
  {
    auto b = s.begin();
    const auto e = s.end();
    while (true) {
      b = std::find_if(b, e, p);
      if (b == e) return; // Too bad end is not erasable as a no-op...
      b = s.erase(b);
    }
  }

  /// ContiguousLikeSequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_dispatch(S& s, const P& p, traits::True isContiguousLike, traits::False isList)
  {
    erase_if_contiguous_like(s, p);
  }

  /// ListSequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_dispatch(S& s, const P& p, traits::False isContiguousLike, traits::True isList)
  {
    erase_if_list(s, p);
  }

  /// Sequence<T> S, Predicate<T> P
  template<typename S, typename P>
  void erase_if_dispatch(S& s, const P& p, traits::False isContiguousLike, traits::False isList)
  {
    erase_if_default(s, p);
  }
} // namespace detail

/// Erase all elements that respect the predicate.
/// Optimizations are performed for contiguous-like sequences (vector, deque...)
/// and for list sequences (list, forward_list).
/// If you want to take advantage on this for your own containers, specialize
/// traits::IsContiguousLike or traits::IsList.
/// Note: It would be nice to return the predicate, because it could be stateful
/// and for example count the number of true elements, but it's not possible because
/// underlying stl algorithms and member functions do not return the predicate...
/// Sequence<T> S, Predicate<T> P
template<typename S, typename P>
void erase_if(S& s, const P& p)
{
  detail::erase_if_dispatch(s, p, traits::IsContiguousLike<S>{}, traits::IsList<S>{});
}

} // namespace qi

#endif  // _QI_ALGORITHM_HPP_

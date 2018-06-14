#ifndef KA_ARK_MUTABLE_HPP
#define KA_ARK_MUTABLE_HPP
#pragma once
#include "../macro.hpp"
#include "../macroregular.hpp"
#include "../src.hpp"
#include "../typetraits.hpp"
#include "../utility.hpp"

namespace ka {

/// Contains archetypes.
///
/// An archetype is a minimal model of a concept. It is a wrapper that restricts
/// the interface of a value to its bare minimum for a given concept.

// TODO: Put this namespace back when get rid of VS2013 (this compiler is not
// able to find free functions defined in namespace `ka` from namespace
// `ka::ark`).
//namespace ark {

  /// Archetype of a Mutable.
  ///
  /// It can be seen as a interface restriction.
  ///
  /// For the `Mutable` concept, see `concept.hpp`.
  ///
  /// Example: ensuring by construction that a pointer cannot be deleted, nor
  ///   be made to point elsewhere
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct node {
  ///   ark_mutable_t<node*> _next;
  ///   ...
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Mutable M
  template<typename M>
  class ark_mutable_t {
    M m_;
  public:
  // Regular:
    ark_mutable_t() : m_{} {
    }
    KA_GENERATE_FRIEND_REGULAR_OPS_1(ark_mutable_t, m_)

  // Custom:
    template<typename T, typename = EnableIfNotBaseOf<ark_mutable_t, T>>
    BOOST_CONSTEXPR
    explicit ark_mutable_t(T&& t) KA_NOEXCEPT_EXPR(M{fwd<T>(t)})
      : m_(fwd<T>(t)) {
    }

  // Mutable:
    // Both const and non-const versions are defined to avoid default template
    // versions to be selected.
    BOOST_CONSTEXPR
    friend auto src(ark_mutable_t& x) KA_NOEXCEPT_EXPR(src(declref<M>())) -> decltype(src(declref<M>())) {
      return src(x.m_);
    }

    BOOST_CONSTEXPR
    friend auto src(ark_mutable_t const& x) KA_NOEXCEPT_EXPR(src(declref<M>()))
        -> decltype(src(declref<M>())) {
      return src(const_cast<M&>(x.m_));
    }
  };

  template<typename M>
  ark_mutable_t<Decay<M>> make_mutable(M&& m) {
    return ark_mutable_t<Decay<M>>{fwd<M>(m)};
  }

// TODO: Put this namespace back when get rid of VS2013.
//}} // ka::ark
}

#endif // KA_ARK_MUTABLE_HPP

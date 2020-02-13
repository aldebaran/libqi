#ifndef KA_FUNCTOR_HPP
#define KA_FUNCTOR_HPP
#pragma once
#include "macro.hpp"
#include "macroregular.hpp"
#include "utility.hpp"
#include "functional.hpp"
#include "typetraits.hpp"

/// Defines the `ka::fmap` function that is part of the `Functor` and
/// `FunctorApp` concepts (see `concept.hpp` for the formal definitions). This
/// function is defined for:
///
/// - standard containers (in functorcontainer.hpp)
/// - optionals (in opt.hpp): boost::optional, ka::opt
/// - functions (in functional.hpp): std::function (boost::function's model
///     could be defined in the same way)
/// - any type that defines a member function `fmap` (in this case `ka::fmap`
///   simply forwards to the member function).
///
/// `ka::fmap` is defined as a polymorphic *function object*, that calls a free
/// function `fmap` through `ADL`. The benefits of this approach are that:
///
/// - the caller always write `ka::fmap` (no `using ka::fmap` needed) while
///   `ADL` is still triggered.
/// - as a function object, `ka::fmap` is easy to manipulate (as a member in
///   another type, through function composition, etc.), in contrast to a
///   template function.
///
/// The general approach is taken from Eric Niebler:
/// http://ericniebler.com/2014/10/21/customization-point-design-in-c11-and-beyond/
///
/// `ka::fmap` can also be partially applied resulting in a polymorphic lifted
/// function (see example below).
///
/// Example: Applying a unary function to the elements of a container.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // vector<human_t> humans;
/// // mood_t mood(human_t const&);
/// vector<mood_t> moods = ka::fmap(mood, humans);
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: Applying a n-ary function to the elements of several containers
///          (`FunctorApp` concept)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // Warning: All containers must have the same number of elements.
/// // vector<human_t> humans;
/// // vector<place_t> places;
/// // memory_t memory_at(human_t const&, place_t const&);
/// vector<memory_t> memories = ka::fmap(memory_at, humans, places);
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: Applying a n-ary function to the value of several optionals
///          (`FunctorApp` concept)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // (Of course, it is also possible to apply the function to a single
/// // optional.)
/// // boost::optional<human_t> detected_human;
/// // boost::optional<message_t> pending_message;
/// // mood_t inform(human_t const&, message_t const&);
///
/// // If any input optional is empty, output optional is empty.
/// boost::optional<mood_t> m = ka::fmap(inform, detected_human, pending_message);
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: Applying a unary function to a function
///          (results in function composition).
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // std::function<human_t (robot_t)> owner;
/// // string name(human_t const&); // Ok with any function object if signatures
/// //                              // match.
/// std::function<string (robot_t)> name_of_owner = ka::fmap(name, owner);
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: Applying the same lifted function to different functors.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// // int age(human_t const&);
/// // ka::opt_t<human_t> detected_human;
/// // std::map<string, human_t> humans_by_name;
/// // std::function<human_t (event_t)> organizer;
/// auto age_lift = ka::fmap(age);
/// ka::opt_t<int> maybe_age = age_lift(detected_human);
/// std::map<string, int> ages_by_name = age_lift(humans_by_name);
/// std::function<int (event_t)> age_of_organizer = age_lift(organizer);
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// TODO: Remove trailing returns when above C++11. Beware of the distinction
//       between `auto` and `decltype(auto)`.

namespace ka {

namespace fmap_ns {
  // Overloads are defined here and available for `fmap_fn_t`.

  // Type has a member fmap: call it.
  // Function<B (A)> F
  template<typename F, typename T>
  auto fmap_dispatch(true_t /* HasMemberFmap<C> */, F&& f, T&& t)
      -> decltype(fwd<T>(t).fmap(fwd<F>(f))) {
    return fwd<T>(t).fmap(fwd<F>(f));
  }

  // Dispatches to the member function if possible.
  // Note: Do not remove the trailing return type as it is needed for SFINAE.
  template<typename F, typename T, typename... U> KA_CONSTEXPR
  auto fmap(F&& f, T&& t, U&&... u)
      -> decltype(fmap_dispatch(HasMemberFmap<Decay<T>>{}, fwd<F>(f), fwd<T>(t), fwd<U>(u)...)) {
    return fmap_dispatch(HasMemberFmap<Decay<T>>{}, fwd<F>(f), fwd<T>(t), fwd<U>(u)...);
  }

  template<typename F>
  struct fmap_bound_t {
    F f;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(fmap_bound_t, f)
  // Function:
    template<typename T, typename... U> KA_CONSTEXPR
    auto operator()(T&& t, U&&... u) const -> decltype(fmap(f, fwd<T>(t), fwd<U>(u)...)) {
      // Performs ADL.
      return fmap(f, fwd<T>(t), fwd<U>(u)...);
    }
  };

  // Must be defined in this namespace to get access to the overloads.
  struct fmap_fn_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(fmap_fn_t)
  // Function<X<B> (F, X<A>...)>, with Function<B (A...)> F:
    template<typename F, typename T, typename... U> KA_CONSTEXPR
    auto operator()(F&& f, T&& t, U&&... u) const -> decltype(fmap(fwd<F>(f), fwd<T>(t), fwd<U>(u)...)) {
      // Performs ADL.
      return fmap(fwd<F>(f), fwd<T>(t), fwd<U>(u)...);
    }

    // Partial application.
    template<typename F> KA_CONSTEXPR
    auto operator()(F&& f) const -> fmap_bound_t<Decay<F>> {
      return {fwd<F>(f)};
    }
  };
} // namespace fmap_ns

/// Function that calls the `fmap` free function through `ADL`.
///
/// Lookup algorithm:
///   if a `fmap` function is found through `ADL`:
///     call it
///   else if the type has a member `fmap`:
///     call it
///   else if `fmap_dispatch(false_t, F, T...)` is defined:
///     call it (if this specialization comes from `functorcontainer.hpp`, a
///       standard container is assumed)
///
using fmap_ns::fmap_fn_t;

// Avoid ODR violations.
namespace {
  static auto const& fmap = static_const_t<fmap_fn_t>::value;
}

} // namespace ka

#endif // KA_FUNCTOR_HPP

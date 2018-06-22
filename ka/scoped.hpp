#ifndef KA_SCOPED_HPP
#define KA_SCOPED_HPP
#pragma once
#include <functional>
#include <utility>
#include "functional.hpp"
#include "macroregular.hpp"
#include "typetraits.hpp"
#include "utility.hpp"

namespace ka {
  /// Execute some code on destruction.
  ///
  /// Useful to do some cleanup in a RAII style when exiting the current scope.
  /// This type is constructed with 2 arguments:
  /// a value and a function to which it will be passed.
  /// The value could be a resource and the function the one to free it, but
  /// other uses are possible.
  ///
  /// Example 1: resource that is released
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///   auto guard = scoped(fopen("path/to/file", "r"), [](FILE* f) {
  ///     if (f) fclose(f);
  ///   });
  ///   auto file = guard.value;
  ///   // code using file...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example 2: non-resource - we want to restore an initial state on scope exit
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///   // setf() sets the new flags and returns the old ones.
  ///   // We set the hex flag and want the initial flags to be restored on scope exit.
  ///   // Here, o is a std::ostream.
  ///   auto s = scoped(o.setf(ios::hex, ios::basefield), [&](ios::fmtflags old) {
  ///     o.setf(old, ios::basefield);
  ///   });
  ///   // code using o...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example 3: non-resource
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///   auto logger = scoped(__FUNCTION__, [](char const* f) {
  ///     std::cout << "Exiting " << f;
  ///   });
  ///   // will log when exiting the scope.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Members are public because there is no invariant to maintain.
  ///
  /// Default construction is allowed because the default constructed value of members
  /// could be meaningful, e.g. if the free function has no state (no member),
  /// and because the member values can be changed after construction.
  ///
  /// No release mechanism is provided because it can be easily implemented inside
  /// the free function, especially if it is a lambda (see the unit test named
  /// "Release" for an example).
  ///
  /// A boolean is used to avoid moved-from objects to call the function in their
  /// destructor. This overhead could be avoided by letting the user decide if
  /// she/he wants a moveable type, via a template parameter. Then, we would
  /// specialize scoped_t to have this boolean only when necessary.
  /// For now, it has been decided not to do so in order to keep the code simple.
  ///
  /// Regular T, FunctionObject<U (T)> F where U is not constrained
  template<typename T, typename F>
  struct scoped_t {
    T value; /// Could be a resource.
    F f; /// Could be a function to free this resource.
  private:
    bool moved_from = false;
  public:

    /// Regular T1, FunctionObject<U (T1)> F1 where
    ///   T1 is convertible to T, F1 is convertible to F
    template<typename T1, typename F1>
    scoped_t(T1&& t1, F1&& f1) : value(fwd<T1>(t1)), f(fwd<F1>(f1)) {
    }
    scoped_t() = default;
    scoped_t(scoped_t const&) = delete;

    // TODO : use "= default" when get rid of vs2013.
    scoped_t(scoped_t&& a) : value(std::move(a.value)), f(std::move(a.f)) {
      a.moved_from = true;
    }

    scoped_t& operator=(scoped_t const&) = delete;
    scoped_t& operator=(scoped_t&& a) = delete;

    ~scoped_t() {
      if (!moved_from) f(std::move(value));
    }
    KA_GENERATE_FRIEND_REGULAR_OPS_2(scoped_t, value, f)
  };

  /// A specialized version is needed for void because here no value is stored
  /// and so the function to call takes no parameter.
  ///
  /// If [void were a regular type](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0146r1.html),
  /// this whole specialization would be unnecessary...
  ///
  /// FunctionObject<U ()> F where U is not constrained
  template<typename F>
  struct scoped_t<void, F> {
    F f;
  private:
    bool moved_from = false;
  public:
    /// We must ensure this cannot be (ab)used as a copy constructor (hence EnableIf).
    template<typename G, typename = EnableIf<!std::is_base_of<scoped_t, G>::value>>
    explicit scoped_t(G&& f)
      : f(fwd<G>(f)) {
    }
    scoped_t() = default;
    scoped_t(scoped_t const&) = delete;

    // TODO : use "= default" when get rid of vs2013.
    scoped_t(scoped_t&& a) : f(std::move(a.f)) {
      a.moved_from = true;
    }

    scoped_t& operator=(scoped_t const&) = delete;
    scoped_t& operator=(scoped_t&&) = delete;

    ~scoped_t() {
      if (!moved_from) f();
    }
    KA_GENERATE_FRIEND_REGULAR_OPS_1(scoped_t, f)
  };

  /// Returns a scoped_t for the given value and function.
  /// Useful to do type deduction.
  ///
  /// Regular T, FunctionObject<U (T)> F where U is not constrained
  template<typename T, typename F>
  scoped_t<Decay<T>, Decay<F>> scoped(T&& value, F&& f) {
    return scoped_t<Decay<T>, Decay<F>>{fwd<T>(value), fwd<F>(f)};
  }

  /// Returns a scoped_t for the given function.
  /// Useful to do type deduction.
  ///
  /// FunctionObject<U (T)> F where U is not constrained
  template<typename F>
  scoped_t<void, Decay<F>> scoped(F&& f) {
    return scoped_t<void, Decay<F>>{fwd<F>(f)};
  }

  /// Applies an action and applies its retraction on scope exit.
  ///
  /// Example: incrementing an atomic counter and decrementing it on scope exit
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto incr = [](std::atomic<int>& x) {++x;};
  /// auto decr = [](std::atomic<int>& x) {--x;};
  ///
  /// // `counter` is a std::atomic<int>
  /// {
  ///   auto _ = scoped_apply_and_retract(counter, incr, decr);
  ///   // here, the counter has been incremented
  ///   // ...
  /// }
  /// // here, the counter has been decremented
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///

  // Contains workarounds for VS2013.
  // TODO: Remove this when get rid of VS2013.
  namespace vs13 {
    template<class F>
    using Retract = typename F::retract_type;
  }

  /// Action<T> F, Action<T> G
  template<typename T, typename F, typename G>
  auto scoped_apply_and_retract(T& value, F&& f, G&& retraction)
  // TODO: Remove the trailing return when get rid of VS2013.
  // A lambda is not used because of the non-evaluated decltype context.
      -> scoped_t<std::reference_wrapper<T>, Decay<G>> {
    // Apply the action now.
    fwd<F>(f)(value);

    // Apply the retraction on scope exit.
    return scoped(std::ref(value), fwd<G>(retraction));
  }

  /// Applies an action and applies its retraction on scope exit.
  ///
  /// The retraction is obtained through the `IsomorphicAction` concept.
  ///
  /// IsomorphicAction<T> F
  template<typename T, typename F>
  auto scoped_apply_and_retract(T& value, F&& f)
  // TODO: 1) Get rid of VS2013
  //       2) Remove the trailing return when we upgrade to C++14 or higher (aka C++14+)
  // A lambda is not used because of the non-evaluated decltype context.
      -> scoped_t<std::reference_wrapper<T>, vs13::Retract<Decay<F>>> {
    return scoped_apply_and_retract(value, fwd<F>(f), retract(f));
  }

  /// Restores a value on scope exit.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// void print(Element const& e) {
  ///   auto _ = scoped_set_and_restore(flags, flag_hex | flag_pretty);
  ///   // Performs:
  ///   //  auto old_flags = flags;
  ///   //  flags = flag_hex | flag_pretty;
  ///   ...
  /// }
  /// // On scope exit, performs:
  /// //  flags = old_flags;
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Remark: Also works with move-only types.
  ///
  /// Warning: As with `scoped` the return value must be stored on the stack.
  ///   If it is not stored, the value is of course immediately restored.
  ///
  /// With T t, U u, the following is valid:
  ///   t = std::move(u);
  template<typename T, typename U>
  auto scoped_set_and_restore(T& value, U&& newValue)
  // TODO: Remove the trailing return when we upgrade to C++14+
  // A lambda is not used because of the non-evaluated decltype context.
    -> decltype(scoped_apply_and_retract(
      value,
      move_assign<T>(fwd<U>(newValue)),
      move_assign<T>(std::move(value)))) {
    return scoped_apply_and_retract(
      value,

      // Will perform: value = std::move(Decay<U>(fwd<U>(newValue)));
      // (c++ is beautiful :D)
      move_assign<T>(fwd<U>(newValue)),

      // Will perform: value = std::move(Decay<T>(std::move(oldValue));
      // `oldValue` is `value` when entering this procedure.
      move_assign<T>(std::move(value)));
  }

  /// Increments a value and decrements it on scope exit.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// void T::invoke_maybe(boost::function<void()> f, const boost::system::error_code& erc) {
  ///   if (!erc) {
  ///     auto _ = scoped_incr_and_decr<Atomic<uint64_t>>(_activeTask);
  ///     // Here, `_activeTask` has been incremented.
  ///     ...
  ///   }
  ///   // Here, `_activeTask` has been decremented.
  ///   ...
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// (Integral || BidirectionalIterator) T
  template<typename T>
  auto scoped_incr_and_decr(T& value)
      -> decltype(scoped_apply_and_retract(value, incr_mono_t<T>{})) {
    return scoped_apply_and_retract(value, incr_mono_t<T>{});
  }
} // namespace ka

#endif // KA_SCOPED_HPP

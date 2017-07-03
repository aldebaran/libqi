#pragma once
#include <qi/functional.hpp>
#include <qi/type/traits.hpp>
#include <qi/macroregular.hpp>

namespace qi
{
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
  ///     qiLogDebug() << "Exiting " << f;
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
  /// specialize Scoped to have this boolean only when necessary.
  /// For now, it has been decided not to do so in order to keep the code simple.
  ///
  /// Regular T, FunctionObject<U (T)> F where U is not constrained
  template<typename T, typename F>
  struct Scoped
  {
    T value; /// Could be a resource.
    F f; /// Could be a function to free this resource.
  private:
    bool _movedFrom = false;
  public:

    /// Regular T1, FunctionObject<U (T1)> F1 where
    ///   T1 is convertible to T, F1 is convertible to F
    template<typename T1, typename F1>
    Scoped(T1&& t1, F1&& f1) : value(std::forward<T1>(t1)), f(std::forward<F1>(f1))
    {
    }
    Scoped() = default;
    Scoped(const Scoped&) = delete;

    // TODO : use "= default" when get rid of vs2013.
    Scoped(Scoped&& a) : value(std::move(a.value)), f(std::move(a.f))
    {
      a._movedFrom = true;
    }

    Scoped& operator=(const Scoped&) = delete;

    // TODO : use "= default" when get rid of vs2013.
    Scoped& operator=(Scoped&& a)
    {
      value = std::move(a.value);
      f = std::move(a.f);
      a._movedFrom = true;
      return *this;
    }

    ~Scoped()
    {
      if (!_movedFrom) f(std::move(value));
    }
    QI_GENERATE_FRIEND_REGULAR_OPS_2(Scoped, value, f)
  };

  /// A specialized version is needed for void because here no value is stored
  /// and so the function to call takes no parameter.
  ///
  /// If [void were a regular type](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0146r1.html),
  /// this whole specialization would be unnecessary...
  ///
  /// FunctionObject<U ()> F where U is not constrained
  template<typename F>
  struct Scoped<void, F>
  {
    F f;
  private:
    bool _movedFrom = false;
  public:
    /// We must ensure this cannot be (ab)used as a copy constructor (hence EnableIf).
    template<typename G, typename = traits::EnableIf<!std::is_base_of<Scoped, G>::value>>
    explicit Scoped(G&& f)
      : f(std::forward<G>(f))
    {
    }
    Scoped() = default;
    Scoped(const Scoped&) = delete;

    // TODO : use "= default" when get rid of vs2013.
    Scoped(Scoped&& a) : f(std::move(a.f))
    {
      a._movedFrom = true;
    }

    Scoped& operator=(const Scoped&) = delete;

    // TODO : use "= default" when get rid of vs2013.
    Scoped& operator=(Scoped&& a)
    {
      f = std::move(a.f);
      a._movedFrom = true;
      return *this;
    }

    ~Scoped()
    {
      if (!_movedFrom) f();
    }
    QI_GENERATE_FRIEND_REGULAR_OPS_1(Scoped, f)
  };

  /// Returns a Scoped for the given value and function.
  /// Useful to do type deduction.
  ///
  /// Regular T, FunctionObject<U (T)> F where U is not constrained
  template<typename T, typename F>
  Scoped<traits::Decay<T>, traits::Decay<F>> scoped(T&& value, F&& f)
  {
    return Scoped<traits::Decay<T>, traits::Decay<F>>{std::forward<T>(value), std::forward<F>(f)};
  }

  /// Returns a Scoped for the given function.
  /// Useful to do type deduction.
  ///
  /// FunctionObject<U (T)> F where U is not constrained
  template<typename F>
  Scoped<void, traits::Decay<F>> scoped(F&& f) {
      return Scoped<void, traits::Decay<F>>{std::forward<F>(f)};
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
  ///   auto _ = scopedApplyAndRetract(counter, incr, decr);
  ///   // here, the counter has been incremented
  ///   // ...
  /// }
  /// // here, the counter has been decremented
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///

  // Contains workarounds for VS2013.
  // TODO: Remove this when get rid of VS2013.
  namespace vs13
  {
    template<class T, class U>
    using Apply = ApplyAction<T, traits::Decay<U>>;

    template<class F>
    using Retract = typename F::retract_type;
  }

  /// Action<T> F, Action<T> G
  template<typename T, typename F, typename G>
  auto scopedApplyAndRetract(T& value, F&& f, G&& retraction)
  // TODO: Remove the trailing return when get rid of VS2013.
  // A lambda is not used because of the non-evaluated decltype context.
    -> Scoped<void, vs13::Apply<T, G>>
  {
    // Apply the action now.
    std::forward<F>(f)(value);

    // Apply the retraction on scope exit.
    return scoped(ApplyAction<T, traits::Decay<G>>{value, std::forward<G>(retraction)});
  }

  /// Applies an action and applies its retraction on scope exit.
  ///
  /// The retraction is obtained through the `IsomorphicAction` concept.
  ///
  /// IsomorphicAction<T> F
  template<typename T, typename F>
  auto scopedApplyAndRetract(T& value, F&& f)
  // TODO: 1) Get rid of VS2013
  //       2) Remove the trailing return when we upgrade to C++14 or higher (aka C++14+)
  // A lambda is not used because of the non-evaluated decltype context.
    -> Scoped<void, vs13::Apply<T, vs13::Retract<F>>>
  {
    return scopedApplyAndRetract(value, std::forward<F>(f), retract(f));
  }

  /// Restores a value on scope exit.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `depth` is an `int` with a wider scope.
  ///
  /// void print(const Element& e)
  /// {
  ///   auto _ = scopedSetAndRestore(depth, depth + 1);
  ///   // Performs:
  ///   //  auto oldDepth = depth;
  ///   //  depth = depth + 1;
  ///   ...
  /// }
  /// // On scope exit, performs:
  /// //  depth = oldDepth;
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
  auto scopedSetAndRestore(T& value, U&& newValue)
  // TODO: Remove the trailing return when we upgrade to C++14+
  // A lambda is not used because of the non-evaluated decltype context.
    -> decltype(scopedApplyAndRetract(
      value,
      makeMoveAssign(std::forward<U>(newValue)),
      makeMoveAssign(std::move(value))))
  {
    return scopedApplyAndRetract(
      value,

      // Will perform: value = std::move(Decay<U>(std::forward<U>(newValue)));
      // (c++ is beautiful :D)
      makeMoveAssign(std::forward<U>(newValue)),

      // Will perform: value = std::move(Decay<T>(std::move(oldValue));
      // `oldValue` is `value` when entering this procedure.
      makeMoveAssign(std::move(value)));
  }

} // namespace qi

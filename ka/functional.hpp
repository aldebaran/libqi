#ifndef KA_FUNCTIONAL_HPP
#define KA_FUNCTIONAL_HPP
#pragma once
#include <functional>
#include <boost/config.hpp>
#include <boost/optional.hpp>
#include "integersequence.hpp"
#include "macro.hpp"
#include "macroregular.hpp"
#include "memory.hpp"
#include "src.hpp"
#include "typetraits.hpp"
#include "utility.hpp"

/// @file
/// Contains functional tools: function composition, constant function, identity
/// function, lock-and-call wrapper, etc.

namespace ka {

  /// Polymorphic function that maps any input to the same output.
  ///
  /// Copyable Ret
  template<typename Ret>
  struct constant_function_t {
    Ret ret;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(constant_function_t, ret)
  // PolymorphicFunction<Ret (Args...)>:
    template<typename... Args>
    Ret operator()(Args const&...) const {
      return ret;
    }
  };

  KA_DERIVE_CTOR_FUNCTION_TEMPLATE(constant_function)

  /// `void` specialization
  template<>
  struct constant_function_t<void> {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(constant_function_t)
  // PolymorphicFunction<void (Args...)>:
    template<typename... Args>
    void operator()(Args const&...) const {
    }
  };

  KA_DERIVE_CTOR_FUNCTION_TEMPLATE_VOID(constant_function)

  /// A polymorphic transformation that takes a value and returns it as-is.
  ///
  /// A transformation is a unary function from a type to itself.
  struct id_transfo_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(id_transfo_t)
  // PolymorphicTransformation:
    template<typename T>
    inline T operator()(T&& t) const {
      return fwd<T>(t);
    }
  // Isomorphism:
    friend id_transfo_t retract(id_transfo_t x) {
      return x;
    }
  };

  KA_DERIVE_CTOR_FUNCTION(id_transfo)

  /// A polymorphic action that does not modify its argument.
  ///
  /// See concept `Action`.
  struct id_action_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(id_action_t)
  // PolymorphicAction:
    template<typename T>
    inline void operator()(T&) const {
    }
  };

  KA_DERIVE_CTOR_FUNCTION(id_action)

// Mainly useful because of the redundancy of trailing return types.
//
// TODO: Remove this when get rid of C++11.
#define KA_FWD_ARGS fwd<Args>(args)...

  namespace detail {
    template<typename Ret>
    struct composition_t {
      /// TODO: Remove the trailing return type when get rid of C++11.
      template<typename G, typename F, typename... Args>
      auto operator()(G& g, F& f, Args&&... args) const -> decltype(g(f(KA_FWD_ARGS))) {
        return g(f(KA_FWD_ARGS));
      }
    };

    template<>
    struct composition_t<void> {
      /// TODO: Remove the trailing return type when get rid of C++11.
      template<typename G, typename F, typename... Args>
      auto operator()(G& g, F& f, Args&&... args) const -> decltype(g()) {
        return f(KA_FWD_ARGS), g();
      }
    };
  } // namespace detail

  /// Composes two procedures, handling the void case.
  ///
  /// In C++, `g(f(x))` is not possible if `f` returns `void` and `g` takes `void`
  /// (no argument). This case is handled as `f(x), g()`.
  ///
  /// Procedure<C (B)> G, Procedure<B (A...)> F
  template<typename G, typename F>
  struct composition_t {
    G g;
    F f;
  // Regular (if G and F are):
    KA_GENERATE_FRIEND_REGULAR_OPS_2(composition_t, g, f)
  // Procedure<C (A...)>:
    /// TODO: Remove the trailing return type when get rid of C++11.
    template<typename... Args>
    auto operator()(Args&&... args)
        -> decltype(detail::composition_t<decltype(f(KA_FWD_ARGS))>{}(g, f, KA_FWD_ARGS)) {
      return detail::composition_t<decltype(f(KA_FWD_ARGS))>{}(g, f, KA_FWD_ARGS);
    }

    template<typename... Args>
    auto operator()(Args&&... args) const
        -> decltype(detail::composition_t<decltype(f(KA_FWD_ARGS))>{}(g, f, KA_FWD_ARGS)) {
      return detail::composition_t<decltype(f(KA_FWD_ARGS))>{}(g, f, KA_FWD_ARGS);
    }
  // RetractableFunction (if `F` and `G` are):
    // TODO: Add it when switching to C++14.
    // implementation is: x -> compose(retract(x.f), retract(x.g))
  };

#undef KA_FWD_ARGS

  namespace detail {
    template<typename G, typename F>
    using IsCompositionIdentity = Conjunction<
      std::is_empty<G>,
      ka::IsRetract<G, F>>;
  } // namespace detail

  /// Performs a mathematical function composition.
  ///
  /// The composition is smart enough so that if you compose a function and its
  /// retraction, it will return the identity function.
  ///
  /// Example: Composing with a non-void function
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `motor` takes a `string` and returns the properties of a motor.
  /// // `heat` takes the properties of a motor and returns an `int`.
  /// // `motor_names` is a container of string.
  /// auto heat_of_motor = compose(heat, motor);
  /// auto max_heat = std::max_element(motor_names.begin(), motor_names.end(), heat_of_motor);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Composing with a void function
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `log_name` and `log_heat` take no input (void) and returns no ouput (void).
  /// auto log_name_then_heat = compose(log_heat, log_name);
  /// auto fut = std::async(log_name_then_heat);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Remark: In the future, add a SFINAE guard if there is a name clash.
  ///   Alternatively, rename this function in `fun_compose`.
  ///
  /// Remark: Operator `*` is proposed as an opt-in alternative to `compose` in
  ///   the `functional_ops` namespace.
  ///
  /// Procedure<C (B)> G,
  /// Procedure<B (Args...)> F

// TODO: Remove this when get rid of VS2013.
#if KA_COMPILER_VS2013_OR_BELOW
  // Poor-man version that does not perform  simplifications.
  template<typename G, typename F>
  composition_t<Decay<G>, Decay<F>> compose(G&& g, F&& f) {
    return {fwd<G>(g), fwd<F>(f)};
  }
#else
  template<typename G, typename F, typename =
    EnableIf<!detail::IsCompositionIdentity<Decay<G>, Decay<F>>::value>>
  composition_t<Decay<G>, Decay<F>> compose(G&& g, F&& f) {
    return {fwd<G>(g), fwd<F>(f)};
  }

  template<typename F, typename = EnableIf<std::is_empty<F>::value>>
  id_transfo_t compose(Retract<Decay<F>> const&, F const&) {
    return {};
  }
#endif

  template<typename F>
  F&& compose(id_transfo_t, F&& f) {
    return fwd<F>(f);
  }

  template<typename F>
  F&& compose(F&& f, id_transfo_t) {
    return fwd<F>(f);
  }

  // This one could be theoretically handled by the simplifying overload
  // (because `id_transfo_t` is its own retraction/inverse). But removing this
  // overload causes overload resolution to fail because of ambiguities.
  inline id_transfo_t compose(id_transfo_t, id_transfo_t) {
    return {};
  }

  /// Composes two accumulations.
  ///
  /// An `Accumulation` has the signature `void (T&, Args...)`, meaning it modifies
  /// its argument in-place. There is always a semantically identical `Function`
  /// of the form `T (T)`.
  ///
  /// Composing two accumulations `g` and `f` is equivalent to perform
  /// ```
  /// f(args...);
  /// g(args...);
  /// ```
  /// This implies that both accumulations must have the same signature to be
  /// composable.
  ///
  /// Example: Arithmetic actions
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// using M = matrix_t<int, 10000, 10000>;
  ///
  /// // We use actions to avoid the expensive copies that would happen with the
  /// // equivalent transformations (at least without compiler optimizations).
  /// incr_t<M> incr;   // signature: void (M&)
  /// twice_t<M> twice; // signature: void (M&)
  ///
  /// // Using `compose` wouldn't work here because we don't pass the output of
  /// // `incr` to the input of `twice`.
  ///
  /// auto twice_of_incr = compose_accu(twice, incr); // signature: void (M&)
  ///
  /// M m = compute_matrix();
  /// twice_of_incr(m); // Apply `incr`, then `twice`.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Remark: Operator `*` is proposed as an opt-in alternative to
  ///   `compose_accu` in the `functional_ops_accu` namespace.
  ///
  /// Accumulation<T, Args...> G, Accumulation<T, Args...> F
  template<typename G, typename F>
  struct composition_accu_t {
    G g;
    F f;
  // Regular (if G and F are):
    KA_GENERATE_FRIEND_REGULAR_OPS_2(composition_accu_t, g, f)
  // Accumulation<T, Args...>:
    template<typename T, typename... Args>
    void operator()(T& t, Args const&... args) {
      f(t, args...);
      g(t, args...);
    }

    template<typename T, typename... Args>
    void operator()(T& t, Args const&... args) const {
      f(t, args...);
      g(t, args...);
    }
  // IsomorphismAccu (if `F` and `G` are):
    // TODO: Add it when switching to C++14.
    // implementation is: x -> compose_accu(retract(x.f), retract(x.g))
  };

  /// Helper-function to deduce types for `composition_accu_t`.
  ///
  /// This function will detect retractions and optimise the composition. This
  /// means that if you compose an action with its retraction (i.e. an action
  /// that undoes it), the identity action will be returned.
  ///
  /// Accumulation<T...> G, Accumulation<T...> F
  template<typename G, typename F, typename = EnableIf<
    !detail::IsCompositionIdentity<Decay<G>, Decay<F>>::value>>
  composition_accu_t<Decay<G>, Decay<F>> compose_accu(G&& g, F&& f) {
    return {fwd<G>(g), fwd<F>(f)};
  }

  template<typename F, typename = EnableIf<std::is_empty<F>::value>>
  id_action_t compose_accu(Retract<F> const&, F const&) {
    return {};
  }

  template<typename F>
  F&& compose_accu(id_action_t, F&& f) {
    return fwd<F>(f);
  }

  template<typename F>
  F&& compose_accu(F&& f, id_action_t) {
    return fwd<F>(f);
  }

  // This one could be theoretically handled by the simplifying overload
  // (because `id_action_t` is its own inverse). But removing this overload causes
  // overload resolution to fail because of ambiguities.
  inline id_action_t compose_accu(id_action_t, id_action_t) {
    return {};
  }

  namespace functional_ops {
    /// Performs a mathematical function composition.
    ///
    /// See `compose()`.
    ///
    /// `*` is used because in mathematics multiplication denotes an
    /// associative operation (not required to be commutative).
    /// Function composition being associative, the composition of `g` with `f`
    /// is often abbreviated as `gf`, reusing the product notation.
    ///
    /// Furthermore `id_transfo_t` being polymorphic, it denotes the identity
    /// functions for all types, so the function composition together with
    /// `id_transfo_t` forms a category.
    ///
    /// Therefore, the following invariants hold:
    ///
    /// ```
    /// // `f` is an arbitrary function.
    /// id_transfo_t _1;
    ///
    /// assert(f * _1 == f);
    /// assert(_1 * f == f);
    /// assert(_1 * _1 == _1);
    /// ```
    template<typename G, typename F>
    auto operator*(G&& g, F&& f) -> decltype(compose(fwd<G>(g), fwd<F>(f))) {
      return compose(fwd<G>(g), fwd<F>(f));
    }

    /// Performs a function composition in reverse order of that of traditional
    /// mathematical function composition, that is `f | g == g * f`, where `f`
    /// is evaluated first, then `g`.
    ///
    /// The motivation for this operator is it can be easier to read in some
    /// situations.
    ///
    /// Example: Composing in place
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// // `motor` takes a `string` and returns the properties of a motor.
    /// // `heat` takes the properties of a motor and returns an `int`.
    /// // `motor_names` is a container of string.
    /// auto max_heat = std::max_element(motor_names.begin(), motor_names.end(),
    ///   motor | heat); // `motor | heat` gets the motor, then its heat
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    template<typename F, typename G>
    auto operator|(F&& f, G&& g) -> decltype(compose(fwd<G>(g), fwd<F>(f))) {
      return compose(fwd<G>(g), fwd<F>(f));
    }
  } // namespace functional_ops

  namespace functional_ops_accu {
    /// Performs a composition of accumulation procedures.
    ///
    /// See `compose_accu()`.
    ///
    /// Note: Symmetrically to `compose` with `id_transfo_t`, `compose_accu` with
    ///   `id_action_t` forms a category. Therefore, the same kind of invariants
    ///   hold:
    ///
    /// ```
    /// // `a` is an arbitrary accumulation.
    /// id_action_t _1;
    ///
    /// assert((a * _1) == a);
    /// assert((_1 * a) == a);
    /// assert((_1 * _1) == _1);
    /// ```
    template<typename G, typename F>
    auto operator*(G&& g, F&& f) -> decltype(compose_accu(fwd<G>(g), fwd<F>(f))) {
      return compose_accu(fwd<G>(g), fwd<F>(f));
    }

    /// Performs a accumulation composition in reverse order of that of traditional
    /// mathematical function composition, that is `f | g == g * f`.
    template<typename F, typename G>
    auto operator|(F&& f, G&& g) -> decltype(compose_accu(fwd<G>(g), fwd<F>(f))) {
      return compose_accu(fwd<G>(g), fwd<F>(f));
    }
  } // namespace functional_ops_accu

  /// Function that transforms the codomain (return type) of the given procedure
  /// into an "enriched" type.
  ///
  /// This is related to monad theory (see below).
  ///
  /// Example: Transforming a procedure of type (A -> B) in a procedure of type (A -> Future<B>):
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `proc` takes an `int` and returns a `bool`
  /// auto f = semilift(proc, UnitFuture{});
  /// Future<bool> fut = f(i);
  /// // ...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// # Background on monads
  ///
  /// A monad can be viewed as an "enriched" type together with functions to go
  /// from and to the corresponding "base" type.
  /// In the above example, `int` is the base type and `Future<int>` is the
  /// enriched, monadic, type.
  ///
  /// To lift is to map a function on base types to a function on the
  /// corresponding enriched types:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// enriched "level":  X<int> ---> X<bool>
  ///                            ^
  ///                            | lift
  ///                            |
  /// base "level":      int    ---> bool
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// In the above case `X` is `Future`.
  ///
  /// Other examples for X are: std::optional, std::vector, etc.
  ///
  /// Generally speaking, the enriched type (e.g. X<int>) gives access to a value
  /// of an underlying type (e.g. int). Thus, it can be viewed as a "container".
  ///
  /// In our case, we have:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// enriched "level":       X<bool>
  ///                         ^    ^
  ///   semilift(proc, unit) /     | unit
  ///                       /      |
  /// base "level":      int ---> bool
  ///                        proc
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// This diagram commutes: all paths are equivalent.
  ///
  /// That is, we have for all x:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// semilift(proc, unit)(x) == compose(unit, proc)(x)
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// where `compose` denotes the function composition.
  ///
  /// `unit` is a function that sends a "base" value to an "enriched", monadic value.
  /// Its name comes from the fact that it is a "unit" or "neutral element" for
  /// monadic composition.
  ///
  /// This function is named `semilift` because it partially "lifts" the procedure:
  /// only the codomain is enriched.
  ///
  /// Procedure Proc
  template<typename Proc, typename F>
  auto semilift(Proc&& p, F&& unit) -> decltype(compose(fwd<F>(unit), fwd<Proc>(p))) {
    return compose(fwd<F>(unit), fwd<Proc>(p));
  }

  /// Bind a data to a procedure without changing its semantics.
  /// One use is to bind a shared pointer to extend the lifetime of some data
  /// related to the procedure.
  ///
  /// This type is a useful equivalent to a variadic generic lambda in C++14:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// T data;
  /// auto data_bound_proc = [data](auto&&... args) {
  ///   // ...
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Warning: This type is not strictly equivalent to a lambda (C++14 or even C++17),
  ///   as it is Regular and lambdas are not.
  ///
  /// Procedure Proc
  template<typename Proc, typename T>
  struct data_bound_proc_t {
    Proc proc;
    T data;
  // Regular (if T is regular):
    KA_GENERATE_FRIEND_REGULAR_OPS_2(data_bound_proc_t, proc, data)
  // Procedure:
    template<typename... Args>
    auto operator()(Args&&... args) -> decltype(proc(fwd<Args>(args)...)) {
      return proc(fwd<Args>(args)...);
    }

    template<typename... Args>
    auto operator()(Args&&... args) const -> decltype(proc(fwd<Args>(args)...)) {
      return proc(fwd<Args>(args)...);
    }
  };

  KA_DERIVE_CTOR_FUNCTION_TEMPLATE(data_bound_proc)

  /// A polymorphic transformation that takes a procedure and returns
  /// an equivalent one that is bound to an arbitrary data.
  /// The bound data can be a shared pointer to extend the lifetime of an object
  /// the procedure is dependent on.
  ///
  /// Example: extending instance lifetime by binding a shared pointer
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Network N
  /// template<typename N>
  /// struct X : std::enable_shared_from_this<X> {
  ///   SendMessageEnqueue<N, SocketPtr<N>> sendMsg;
  ///
  ///   void doStuff() {
  ///     // ...
  ///     auto onSent = [](...) {
  ///        // use X's members here
  ///     };
  ///     // sendMsg will wrap any asynchronous task by binding it with a
  ///     // shared pointer on this instance.
  ///     sendMsg(msg, ssl, onSent,
  ///       data_bound_transfo(shared_from_this()) // lifetimeTransfo
  ///     );
  ///   }
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Regular T
  template<typename T>
  struct data_bound_transfo_t {
    T data;
  // Regular (if T is regular):
    KA_GENERATE_FRIEND_REGULAR_OPS_1(data_bound_transfo_t, data)
  // PolymorphicTransformation:
    template<typename Proc>
    data_bound_proc_t<Decay<Proc>, T> operator()(Proc&& p) {
      return {fwd<Proc>(p), data};
    }

    template<typename Proc>
    data_bound_proc_t<Decay<Proc>, T> operator()(Proc&& p) const {
      return {fwd<Proc>(p), data};
    }
  };

  KA_DERIVE_CTOR_FUNCTION_TEMPLATE(data_bound_transfo)

  /// Action that move-assigns a stored value on call.
  ///
  /// Warning: Can be called only once because the value assigned from is moved.
  ///
  /// With Src s, Dest d, the following is valid:
  ///   d = std::move(s);
  template<typename Dest, typename Src>
  struct move_assign_t {
    Src s;
  // Action<T>:
    /// Precondition: it is the first time this operator is called on this instance
    void operator()(Dest& d) {
      d = std::move(s);
    }

    // TODO: Remove this when get rid of VS2013.
    move_assign_t(Src&& s)
      : s(std::move(s)) {
    }

    // TODO: Remove this when get rid of VS2013.
    move_assign_t(Src const& s)
      : s(s) {
    }

    // TODO: Remove this when get rid of VS2013.
    move_assign_t(move_assign_t const& other) = default;

    // TODO: Remove this when get rid of VS2013.
    move_assign_t(move_assign_t&& x)
      : s(std::move(x.s)) {
    }
  };

  /// Helper function that performs type deduction for `MoveAssign`.
  template<typename Dest, typename Src>
  move_assign_t<Dest, Decay<Src>> move_assign(Src&& s) {
    return std::move(move_assign_t<Dest, Decay<Src>>(fwd<Src>(s)));
  }

  template<typename T>
  struct incr_mono_t;

  /// Isomorphic action that decrements its parameter.
  ///
  /// See `incr_mono_t` for a use example.
  ///
  /// The inverse action is `incr_mono_t<T>`
  ///
  /// (Arithmetic || BidirectionalIterator) T
  template<typename T>
  struct decr_mono_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(decr_mono_t)
  // Action<Arithmetic || BidirectionalIterator>:
    // TODO: Remove this when get rid of VS2013.
    using retract_type = incr_mono_t<T>;

    void operator()(T& x) const {
      --x;
    }
  // IsomorphicAction<Integral || BidirectionalIterator>:
    template<typename U>
    friend incr_mono_t<U> retract(decr_mono_t<U>);
  };

  /// Isomorphic action that increments its parameter.
  ///
  /// Example: Using in conjunction with scoped tools
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// void pretty_print(doc_t const& doc, std::ostream& o) {
  ///   // `depth` is an `int` with a wider scope.
  ///   auto _ = scoped_apply_and_retract(depth, incr_mono_t<int>{});
  ///   // here, the depth has been incremented
  ///   ...
  /// }
  /// // here, the depth has been decremented
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// The inverse action is `decr_mono_t<T>`.
  ///
  /// (Arithmetic || InputIterator) T
  template<typename T>
  struct incr_mono_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(incr_mono_t)
  // Action<Arithmetic || InputIterator>:
    void operator()(T& x) const {
      ++x;
    }
  // IsomorphicAction<Integral || BidirectionalIterator>:
    // TODO: Remove this when get rid of VS2013.
    using retract_type = decr_mono_t<T>;

    friend decr_mono_t<T> retract(incr_mono_t) {
      return {};
    }
  };

  template<typename T>
  incr_mono_t<T> retract(decr_mono_t<T>) {
    return {};
  }

  struct incr_t;

  struct decr_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(decr_t)
  // Action<Arithmetic || BidirectionalIterator>:
    // TODO: Remove this when get rid of VS2013.
    using retract_type = incr_t;

    template<typename T>
    inline void operator()(T& x) const {
      --x;
    }
  // IsomorphicAction<Integral || BidirectionalIterator>:
    inline friend incr_t retract(decr_t);
  };

  KA_DERIVE_CTOR_FUNCTION(decr)

  struct incr_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(incr_t)
  // Action<Arithmetic || InputIterator>:
    template<typename T>
    inline void operator()(T& x) const {
      ++x;
    }
  // IsomorphicAction<Integral || BidirectionalIterator>:
    // TODO: Remove this when get rid of VS2013.
    using retract_type = decr_t;

    inline friend decr_t retract(incr_t) {
      return {};
    }
  };

  inline incr_t retract(decr_t) {
    return {};
  }

  KA_DERIVE_CTOR_FUNCTION(incr)

  namespace detail {
    template<typename Proc, typename Args, std::size_t... I>
    BOOST_CONSTEXPR auto apply_impl(Proc&& proc, Args&& args, index_sequence<I...>)
      // TODO: Guess what, remove this when c++14 is available.
        -> decltype(proc(std::get<I>(fwd<Args>(args))...)) {
      return proc(std::get<I>(fwd<Args>(args))...);
    }
  }

  /// Applies the procedure after unpacking the arguments.
  ///
  /// Besides std::tuple, arguments can be stored as std::pair, std::array, or
  /// any type modeling the `Tuple` concept (see concept.hpp).
  ///
  /// Note: This is roughly equivalent to C++17's `std::apply`. The main difference
  /// is that this version use function call syntax instead of C++17's `std::invoke`.
  ///
  /// TODO: Replace this by `std::apply` when C++17 is available.
  ///
  /// Example: using a `std::tuple` as a container for arguments
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int f(int i, char c, float f) {
  ///   ...
  /// }
  ///
  /// // ...
  /// auto args = std::make_tuple(5, 'a', 3.14f);
  /// int res = apply(f, args);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: using a `std::pair` as a container for arguments
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int f(int i, char c) {
  ///   ...
  /// }
  ///
  /// // ...
  /// auto args = std::make_pair(5, 'a');
  /// int res = apply(f, args);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: using a `std::array` as a container for arguments
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int f(int i, int j, int k, int l) {
  ///   ...
  /// }
  ///
  /// // ...
  /// std::array<int, 4> args = {13, 2, 9874, 42};
  /// int res = apply(f, args);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: using a custom type as a container for arguments
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// namespace test {
  ///   template<std::size_t N>
  ///   using index = std::integral_constant<std::size_t, N>;
  ///
  ///   template<typename A, typename B, typename C>
  ///   struct X {
  ///     A a;
  ///     B b;
  ///     C c;
  ///
  ///     bool operator==(X const& y) const {
  ///       return a == y.a && b == y.b && c == y.c;
  ///     }
  ///
  ///     A& get(index<0>) {return a;}
  ///     B& get(index<1>) {return b;}
  ///     C& get(index<2>) {return c;}
  ///     A const& get(index<0>) const {return a;}
  ///     B const& get(index<1>) const {return b;}
  ///     C const& get(index<2>) const {return c;}
  ///   };
  /// } // namespace test
  ///
  /// namespace std {
  ///   template<typename A, typename B, typename C>
  ///   struct tuple_size<test::X<A, B, C>> : integral_constant<size_t, 3> {
  ///   };
  ///
  ///   template<size_t I, typename A, typename B, typename C>
  ///   BOOST_CONSTEXPR auto get(test::X<A, B, C>& x)
  ///     // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
  ///       -> decltype(x.get(integral_constant<size_t, I>{})) {
  ///     return x.get(integral_constant<size_t, I>{});
  ///   }
  ///
  ///   template<size_t I, typename A, typename B, typename C>
  ///   BOOST_CONSTEXPR auto get(test::X<A, B, C> const& x)
  ///     // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
  ///       -> decltype(x.get(integral_constant<size_t, I>{})) {
  ///     return x.get(integral_constant<size_t, I>{});
  ///   }
  /// } // namespace std
  ///
  /// int f(int i, char c, float f) {
  ///   ...
  /// }
  /// // ...
  ///
  /// X const args{5, 'a', 3.14f};
  /// int res = apply(f, args);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// See `apply_t` to transform a function into an equivalent tuple-accepting function.
  ///
  /// Procedure Proc, Tuple Args
  template<typename Proc, typename Args>
  BOOST_CONSTEXPR auto apply(Proc&& proc, Args&& args)
    // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
      -> decltype(detail::apply_impl(fwd<Proc>(proc), fwd<Args>(args),
        make_index_sequence<std::tuple_size<Decay<Args>>::value>{})) {
    return detail::apply_impl(fwd<Proc>(proc), fwd<Args>(args),
      make_index_sequence<std::tuple_size<Decay<Args>>::value>{});
  }

  /// Procedure accepting a tuple of arguments and unpacking them for the underlying
  /// procedure.
  ///
  /// Example: transforming a "n-argument function" into a "tuple function"
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int f(int i, int j) {
  ///   ...
  /// };
  ///
  /// // ...
  /// auto f2 = apply(f); // helper function that returns a `apply_t<Decay<decltype(f)>>`
  /// int res0 = f2(std::make_tuple(4454, 7));
  /// int res1 = f2(std::make_pair(5, 12));
  /// int res2 = f2(std::array<int, 2>{98, 99});
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Note: See `apply` for an example on how to use a custom type.
  ///
  /// See `apply` for more examples.
  ///
  /// Procedure Proc
  template<typename Proc>
  struct apply_t {
    Proc proc;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(apply_t, proc)
  // Procedure:
    template<typename Args>
    auto operator()(Args&& args) KA_NOEXCEPT_EXPR(apply(proc, fwd<Args>(args)))
        // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
        -> decltype(apply(proc, fwd<Args>(args))) {
      return apply(proc, fwd<Args>(args));
    }

    template<typename Args>
    auto operator()(Args&& args) const KA_NOEXCEPT_EXPR(apply(proc, fwd<Args>(args)))
        // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
        -> decltype(apply(proc, fwd<Args>(args))) {
      return apply(proc, fwd<Args>(args));
    }
  };

  /// Helper function to enable type deduction for `apply_t`.
  ///
  /// Note: An overload taking the procedure _and_ the arguments also exists.
  /// This allows for currying.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int f(int i, int j) {
  ///   ...
  /// };
  ///
  /// // ...
  /// auto args = std::make_pair(34, 45);
  ///
  /// // immediate call version
  /// auto res0 = apply(f, args);
  ///
  /// // currified version (this version)
  /// auto f2 = apply(f);
  /// auto res1 = f2(args);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Procedure Proc
  template<typename Proc>
  apply_t<Decay<Proc>> apply(Proc&& proc) {
    return {fwd<Proc>(proc)};
  }

  // We can't include `opt.hpp` because of the circular dependency with this file. Instead we just
  // forward declare it and include `opt.hpp` at the end of this file.
  template<typename>
  class opt_t;

  namespace detail {
    /// Procedure<T (...)> Proc,
    /// Mutable<ScopeLockable> M
    template <typename Proc, typename M, typename... Args>
    opt_t<ResultOf<Proc&(Args&&...)>> scope_lock_invoke(Proc& proc, M& mut_lockable, Args&&... args) {
      opt_t<ResultOf<Proc&(Args&&...)>> res;
      if (auto lock = scopelock(*mut_lockable)) {
        res.call_set(proc, fwd<Args>(args)...);
      }
      return res;
    }
  }

  /// Procedure wrapper that calls its underlying procedure only if the associated
  /// lockable could be locked and returns an opt_t set with the result of the procedure if it was
  /// called or empty otherwise.
  ///
  /// Example: weak_ptr as a lockable to perform lifetime protection
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // std::shared_ptr<T> p;
  /// async(scope_lock_proc(
  ///   [](int i) mutable { // procedure to be called
  ///     return p->do_stuff(i);
  ///   },
  ///   mutable_store(std::weak_ptr<T>{p}) // lockable: will create a non-null shared_ptr on success
  ///   )
  /// );
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Procedure<T (...)> Proc,
  /// Mutable<ScopeLockable> M
  template<typename Proc, typename M>
  struct scope_lock_proc_t {
    Proc proc;
    M mut_lockable;
  // Regular (if members are):
    KA_GENERATE_FRIEND_REGULAR_OPS_2(scope_lock_proc_t, proc, mut_lockable)

  // Procedure:
    template<typename... Args>
    auto operator()(Args&&... args)
      // TODO: Remove this when we can use C++14
      -> decltype(detail::scope_lock_invoke(proc, mut_lockable, fwd<Args>(args)...)) {
      return detail::scope_lock_invoke(proc, mut_lockable, fwd<Args>(args)...);
    }

    template<typename... Args>
    auto operator()(Args&&... args) const
      // TODO: Remove this when we can use C++14
      -> decltype(detail::scope_lock_invoke(proc, mut_lockable, fwd<Args>(args)...)) {
      return detail::scope_lock_invoke(proc, mut_lockable, fwd<Args>(args)...);
    }
  };

  KA_DERIVE_CTOR_FUNCTION_TEMPLATE(scope_lock_proc)

  /// A polymorphic transformation constructed from an instance `l` of a
  /// ScopeLockable type that takes a procedure and returns an equivalent one
  /// that calls that procedure only if l was successfully locked and keeps the
  /// lock alive until the procedure returns.
  ///
  /// Example: mutex as a lockable to protect a asynchronous function call
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int global_count = 0;
  /// void increment_global_count() {
  ///   ++global_count;
  /// }
  ///
  /// template<typename P>
  /// void async_increment_ten_times(P sync_transfo) {
  ///   for (int i = 0; i < 10; ++i)
  ///     my::async(sync_transfo(increment_global_count));
  /// }
  ///
  /// void do_stuff() {
  ///   static std::mutex m;
  ///   async_increment_ten_times(scope_lock_transfo(&m));
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Mutable<ScopeLockable> M
  template<typename M>
  struct scope_lock_transfo_t {
    M mut_lockable;
  // Regular (if members are regular):
    KA_GENERATE_FRIEND_REGULAR_OPS_1(scope_lock_transfo_t, mut_lockable)
  // PolymorphicTransformation:
    /// Procedure<T (...)> Proc0
    template<typename Proc>
    scope_lock_proc_t<Decay<Proc>, M> operator()(Proc&& p) {
      return { fwd<Proc>(p), mut_lockable };
    }

    /// Procedure<T (...)> Proc0
    template<typename Proc>
    scope_lock_proc_t<Decay<Proc>, M> operator()(Proc&& p) const {
      return { fwd<Proc>(p), mut_lockable };
    }
  };

  KA_DERIVE_CTOR_FUNCTION_TEMPLATE(scope_lock_transfo)

  namespace detail {
  // model EmptyProcedure std::function<A (B...)>:
    template<typename A, typename... B> KA_CONSTEXPR
    bool empty(std::function<A (B...)> const& x) {
      return !static_cast<bool>(x);
    }
  } // namespace detail

  namespace fmap_ns {
    // std::function<_ (X...)> almost models `Functor`, except it is not
    // `Regular` (equality is missing). `fmap` is nonetheless implemented as it
    // can still be useful.

    /// Transforms a function `X... -> A` to `X... -> B`, by post-composing the
    /// function `A -> B`.
    ///
    /// Function<B (A)> F
    template<typename F, typename A, typename... X>
    auto fmap(F&& f, std::function<A (X...)> const& g)
        -> std::function<CodomainFor<F, A> (X...)> {
      using B = CodomainFor<F, A>;
      return std::function<B (X...)>(compose(fwd<F>(f), g));
    }
  } // namespace fmap_ns

  /// Contructs a tuple from the given values.
  /// This is to uniformize with other kinds of product (iterators, etc.).
  template<typename... A>
  auto product(A&&... a) -> decltype(std::make_tuple(fwd<A>(a)...)) {
    return std::make_tuple(fwd<A>(a)...);
  }

  /// Operators related to the category of types and functions.
  namespace fn_cat_ops {
    /// Returns the product of two values.
    template<typename T, typename U,
      typename = EnableIfNotInputIterator<Decay<T>>,
      typename = EnableIfNotInputIterator<Decay<U>>
    >
    auto operator*(T&& t, U&& u) -> decltype(product(fwd<T>(t), fwd<U>(u))) {
      return product(fwd<T>(t), fwd<U>(u));
    }
  } // namespace fn_cat_ops
} // namespace ka

// For ka::scope_lock_proc_t
#include "opt.hpp"

#endif // KA_FUNCTIONAL_HPP

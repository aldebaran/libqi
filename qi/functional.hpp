#pragma once
#include <boost/config.hpp>
#include <boost/optional.hpp>
#include <qi/type/integersequence.hpp>
#include <qi/type/traits.hpp>
#include <qi/utility.hpp>
#include <qi/macroregular.hpp>

/// @file
/// Contains functional tools: no-op procedure, etc.

namespace qi {
  /// A procedure that does nothing.
  template<typename F>
  struct NoOpProcedure;

  /// You can specify the value to return.
  template<typename Ret, typename... Args>
  struct NoOpProcedure<Ret (Args...)>
  {
    Ret _ret;
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_1(NoOpProcedure, _ret)
  // Procedure:
    Ret operator()(const Args&...) const
    {
      return _ret;
    }
  };

  template<typename... Args>
  struct NoOpProcedure<void (Args...)>
  {
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_0(NoOpProcedure)
  // Procedure:
    void operator()(const Args&...) const
    {
    }
  };

  /// Polymorphic function that maps any input to the same output.
  ///
  /// There is no constraint on Ret.
  template<typename Ret>
  struct PolymorphicConstantFunction
  {
    Ret ret;
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_1(PolymorphicConstantFunction, ret)
  // PolymorphicFunction<Ret (Args...)>:
    template<typename... Args>
    Ret operator()(const Args&...) const
    {
      return ret;
    }
  };

  template<>
  struct PolymorphicConstantFunction<void>
  {
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_0(PolymorphicConstantFunction)
  // PolymorphicFunction<void (Args...)>:
    template<typename... Args>
    void operator()(const Args&...) const
    {
    }
  };

  /// A polymorphic transformation that takes a procedure and returns it as-is.
  ///
  /// A transformation is a unary function from a type to itself.
  struct IdTransfo
  {
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_0(IdTransfo)
  // PolymorphicTransformation:
    template<typename T>
    inline T operator()(const T& t) const
    {
      return t;
    }
  };

// Mainly useful because of the redundancy of trailing return types.
//
// TODO: Remove this when get rid of C++11.
#define QI_FWD_ARGS std::forward<Args>(args)...

  namespace detail
  {
    template<typename Ret>
    struct Composition
    {
      /// TODO: Remove the trailing return type when get rid of C++11.
      template<typename G, typename F, typename... Args>
      auto operator()(G& g, F& f, Args&&... args) -> decltype(g(f(QI_FWD_ARGS)))
      {
        return g(f(QI_FWD_ARGS));
      }
    };

    template<>
    struct Composition<void>
    {
      /// TODO: Remove the trailing return type when get rid of C++11.
      template<typename G, typename F, typename... Args>
      auto operator()(G& g, F& f, Args&&... args) -> decltype(g())
      {
        return f(QI_FWD_ARGS), g();
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
  struct Composition
  {
    G g;
    F f;
  // Regular (if G and F are):
    QI_GENERATE_FRIEND_REGULAR_OPS_2(Composition, g, f)
  // Procedure<C (A...)>:
    /// TODO: Remove the trailing return type when get rid of C++11.
    template<typename... Args>
    auto operator()(Args&&... args)
      -> decltype(detail::Composition<decltype(f(QI_FWD_ARGS))>{}(g, f, QI_FWD_ARGS))
    {
      return detail::Composition<decltype(f(QI_FWD_ARGS))>{}(g, f, QI_FWD_ARGS);
    }
  };

  /// Performs a mathematical function composition.
  ///
  /// Example: Composing with a non-void function
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `motor` takes a `string` and returns the properties of a motor.
  /// // `heat` takes the properties of a motor and returns an `int`.
  /// // `motorNames` is a container of string.
  /// auto heatOfMotor = compose(heat, motor);
  /// auto maxHeat = std::max_element(motorNames.begin(), motorNames.end(), heatOfMotor);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Composing with a void function
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `logName` and `logHeat` take no input (void) and returns no ouput (void).
  /// auto logNameThenHeat = compose(logHeat, logName);
  /// auto fut = strand.async(logNameThenHeat);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Remark: In the future, add a SFINAE guard if there is a name clash.
  ///   Alternatively, rename this function in `funCompose`.
  ///
  /// Procedure<C (B)> G,
  /// Procedure<B (Args...)> F
  template<typename G, typename F>
  Composition<traits::Decay<G>, traits::Decay<F>> compose(G&& g, F&& f)
  {
    return {std::forward<G>(g), std::forward<F>(f)};
  }
#undef QI_FWD_ARGS

  /// Function that transform the codomain (return type) of the given procedure
  /// into an "enriched" type.
  ///
  /// This is related to monad theory (see below).
  ///
  /// Example: Transforming a procedure of type (A -> B) in a procedure of type (A -> Future<B>):
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `proc` takes an `int` and returns a `bool`
  /// auto f = semiLift(UnitFuture{}, proc);
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
  ///   semiLift(proc, unit) /     | unit
  ///                       /      |
  /// base "level":      int ---> bool
  ///                        proc
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// This diagram commutes: all paths are equivalent.
  ///
  /// That is we have, for all x:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// semiLift(proc, unit)(x) == (unit . proc)(x)
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// where `.` stands for the function composition (see `compose`).
  ///
  /// `unit` is a function that sends a "base" value to an "enriched", monadic value.
  /// Its name comes from the fact that it is a "unit" or "neutral element" for
  /// monadic composition.
  ///
  /// This function is named `semiLift` because it partially "lifts" the procedure:
  /// only the codomain is enriched.
  ///
  /// Procedure Proc
  template<typename Proc, typename F>
  auto semiLift(Proc&& p, F&& unit)
    -> decltype(compose(std::forward<F>(unit), std::forward<Proc>(p)))
  {
    return compose(std::forward<F>(unit), std::forward<Proc>(p));
  }

  /// Bind a data to a procedure without changing its semantics.
  /// One use is to bind a shared pointer to extend the lifetime of some data
  /// related to the procedure.
  ///
  /// This type is useful equivalent to a variadic generic lambda in C++14:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// T data;
  /// auto dataBoundProc = [data](auto&&... args) {
  ///   // ...
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Warning: This type is not strictly equivalent to a lambda (C++14 or even C++17),
  ///   as it is regular and lambdas are not.
  ///
  /// Procedure Proc
  template<typename Proc, typename T>
  struct DataBoundProc
  {
    Proc _proc;
    T _data;
  // Regular (if T is regular):
    QI_GENERATE_FRIEND_REGULAR_OPS_2(DataBoundProc, _proc, _data)
  // Procedure:
    template<typename... Args>
    auto operator()(Args&&... args) -> decltype(_proc(std::forward<Args>(args)...))
    {
      return _proc(std::forward<Args>(args)...);
    }
  };

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
  ///   SendMessageEnqueue<N, SocketPtr<N>> _sendMsg;
  ///
  ///   void doStuff() {
  ///     // ...
  ///     auto onSent = [](...) {
  ///        // use X's members here
  ///     };
  ///     // _sendMsg will wrap any asynchronous task by binding it with a
  ///     // shared pointer on this instance.
  ///     _sendMsg(msg, ssl, onSent,
  ///       dataBoundTransfo(shared_from_this()) // lifetimeTransfo
  ///     );
  ///   }
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Regular T
  template<typename T>
  struct DataBoundTransfo
  {
    T _data;
  // Regular (if T is regular):
    QI_GENERATE_FRIEND_REGULAR_OPS_1(DataBoundTransfo, _data)
  // PolymorphicTransformation:
    template<typename Proc>
    DataBoundProc<traits::Decay<Proc>, T> operator()(Proc&& p) const
    {
      return {std::forward<Proc>(p), _data};
    }
  };

  /// Helper function to deduce types for DataBoundTransfo.
  template<typename T>
  DataBoundTransfo<T> dataBoundTransfo(const T& maintainAlive)
  {
    return {maintainAlive};
  }

  /// Action that move-assigns a stored value on call.
  ///
  /// Warning: Can be called only once because the value assigned from is moved.
  ///
  /// With Src s, Dest d, the following is valid:
  ///   d = std::move(s);
  template<typename Dest, typename Src>
  struct MoveAssign
  {
    Src s;
  // Action<T>:
    /// Precondition: it is the first time this operator is called on this instance
    void operator()(Dest& d)
    {
      d = std::move(s);
    }

    // TODO: Remove this when get rid of VS2013.
    MoveAssign(Src&& s)
      : s(std::move(s))
    {}

    // TODO: Remove this when get rid of VS2013.
    MoveAssign(const Src& s)
      : s(s)
    {}

    // TODO: Remove this when get rid of VS2013.
    MoveAssign(const MoveAssign& other) = default;

    // TODO: Remove this when get rid of VS2013.
    MoveAssign(MoveAssign&& x)
      : s(std::move(x.s))
    {}
  };

  /// Helper function that performs type deduction for `MoveAssign`.
  template<typename Dest, typename Src>
  MoveAssign<Dest, traits::Decay<Src>> makeMoveAssign(Src&& s)
  {
    return std::move(MoveAssign<Dest, traits::Decay<Src>>(std::forward<Src>(s)));
  }

  template<typename T>
  struct Incr;

  /// Isomorphic action that decrements its parameter.
  ///
  /// See Incr for a use example.
  ///
  /// The inverse action is `Incr<T>`
  ///
  /// (Arithmetic || BidirectionalIterator) T
  template<typename T>
  struct Decr
  {
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_0(Decr)
  // Action<Arithmetic || BidirectionalIterator>:
    // TODO: Remove this when get rid of VS2013.
    using retract_type = Incr<T>;

    void operator()(T& x) const
    {
      --x;
    }
  // IsomorphicAction<Integral || BidirectionalIterator>:
    template<typename U>
    friend Incr<U> retract(Decr<U>);
  };

  /// Isomorphic action that increments its parameter.
  ///
  /// Example: with scoped tools
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// void prettyPrint(const Doc& doc, std::ostream& o)
  /// {
  ///   // `depth` is an `int` with a wider scope.
  ///   auto _ = scopedApplyAndRetract(depth, Incr<int>{});
  ///   // here, the depth has been incremented
  ///   ...
  /// }
  /// // here, the depth has been decremented
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// The inverse action is `Decr<T>`.
  ///
  /// (Arithmetic || InputIterator) T
  template<typename T>
  struct Incr
  {
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_0(Incr)
  // Action<Arithmetic || InputIterator>:
    void operator()(T& x) const
    {
      ++x;
    }
  // IsomorphicAction<Integral || BidirectionalIterator>:
    // TODO: Remove this when get rid of VS2013.
    using retract_type = Decr<T>;

    friend Decr<T> retract(Incr)
    {
      return {};
    }
  };

  template<typename T>
  Incr<T> retract(Decr<T>)
  {
    return {};
  }

  namespace detail
  {
    template<typename Proc, typename Args, std::size_t... I>
    BOOST_CONSTEXPR auto applyImpl(Proc&& proc, Args&& args, qi::index_sequence<I...>)
      // TODO: Guess what, remove this when c++14 is available.
      -> decltype(proc(std::get<I>(std::forward<Args>(args))...))
    {
      return proc(std::get<I>(std::forward<Args>(args))...);
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
  /// namespace test
  /// {
  ///   template<std::size_t N>
  ///   using index = std::integral_constant<std::size_t, N>;
  ///
  ///   template<typename A, typename B, typename C>
  ///   struct X
  ///   {
  ///     A a;
  ///     B b;
  ///     C c;
  ///
  ///     bool operator==(const X& y) const
  ///     {
  ///       return a == y.a && b == y.b && c == y.c;
  ///     }
  ///
  ///     A& get(index<0>) {return a;}
  ///     B& get(index<1>) {return b;}
  ///     C& get(index<2>) {return c;}
  ///     const A& get(index<0>) const {return a;}
  ///     const B& get(index<1>) const {return b;}
  ///     const C& get(index<2>) const {return c;}
  ///   };
  /// } // namespace test
  ///
  /// namespace std
  /// {
  ///   template<typename A, typename B, typename C>
  ///   struct tuple_size<test::X<A, B, C>> : integral_constant<size_t, 3>
  ///   {
  ///   };
  ///
  ///   template<size_t I, typename A, typename B, typename C>
  ///   BOOST_CONSTEXPR auto get(test::X<A, B, C>& x)
  ///     // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
  ///     -> decltype(x.get(integral_constant<size_t, I>{}))
  ///   {
  ///     return x.get(integral_constant<size_t, I>{});
  ///   }
  ///
  ///   template<size_t I, typename A, typename B, typename C>
  ///   BOOST_CONSTEXPR auto get(const test::X<A, B, C>& x)
  ///     // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
  ///     -> decltype(x.get(integral_constant<size_t, I>{}))
  ///   {
  ///     return x.get(integral_constant<size_t, I>{});
  ///   }
  /// } // namespace std
  ///
  /// int f(int i, char c, float f) {
  ///   ...
  /// }
  /// // ...
  ///
  /// const X args{5, 'a', 3.14f};
  /// int res = apply(f, args);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// See `Apply` to transform a function into an equivalent tuple-accepting function.
  ///
  /// Procedure Proc, Tuple Args
  template<typename Proc, typename Args>
  BOOST_CONSTEXPR auto apply(Proc&& proc, Args&& args)
    // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
    -> decltype(detail::applyImpl(fwd<Proc>(proc), fwd<Args>(args),
      make_index_sequence<std::tuple_size<traits::Decay<Args>>::value>{}))
  {
    return detail::applyImpl(fwd<Proc>(proc), fwd<Args>(args),
      make_index_sequence<std::tuple_size<traits::Decay<Args>>::value>{});
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
  /// auto f2 = apply(f); // helper function that returns a `Apply<Decay<decltype(f)>>`
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
  struct Apply
  {
    Proc proc;
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_1(Apply, proc)
  // Procedure:
    template<typename Args>
    auto operator()(Args&& args) QI_NOEXCEPT_EXPR(apply(proc, fwd<Args>(args)))
      // TODO: replace the trailing return by a `decltype(auto)` when c++14 is available
      -> decltype(apply(proc, fwd<Args>(args)))
    {
      return apply(proc, fwd<Args>(args));
    }
  };

  /// Helper function to enable type deduction for `Apply`.
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
  Apply<traits::Decay<Proc>> apply(Proc&& proc)
  {
    return {fwd<Proc>(proc)};
  }

  namespace detail
  {
    template<typename Ret>
    struct ScopeLockProc
    {
      template<typename Proc, typename L, typename... Args>
      boost::optional<Ret> operator()(Proc& proc, L& lockable, Args&&... args) const
      {
        if (auto lock = scopelock(lockable))
        {
          return proc(fwd<Args>(args)...);
        }
        return {};
      }
    };

    template<>
    struct ScopeLockProc<void>
    {
      template<typename Proc, typename L, typename... Args>
      void operator()(Proc& proc, L& lockable, Args&&... args) const
      {
        if (auto lock = scopelock(lockable))
        {
          proc(fwd<Args>(args)...);
        }
      }
    };
  } // namespace detail

  /// Procedure wrapper that calls its underlying procedure only if the associated
  /// lockable could be locked.
  ///
  /// Example: weak_ptr as a lockable to perform lifetime protection
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // std::shared_ptr<T> p;
  /// async(scopeLockProc(
  ///   [](int i) mutable { // procedure to be called
  ///     return p->doStuff(i);
  ///   },
  ///   makeMutableStore(std::weak_ptr<T>{p}) // lockable: will create a non-null shared_ptr on success
  ///   )
  /// );
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Procedure<T (...)> Proc,
  /// Mutable<ScopeLockable> M
  template<typename Proc, typename M>
  struct ScopeLockProc
  {
    Proc _proc;
    M _mutLockable;
  // Regular (if members aremutex.hpp>
    QI_GENERATE_FRIEND_REGULAR_OPS_2(ScopeLockProc, _proc, _mutLockable)
  // Procedure:
    template<typename... Args>
    auto operator()(Args&&... args)
      -> decltype(detail::ScopeLockProc<traits::Decay<decltype(_proc(fwd<Args>(args)...))>>{}
                    (_proc, *_mutLockable, fwd<Args>(args)...)) // TODO: Remove this when we can use C++14
    {
      return detail::ScopeLockProc<traits::Decay<decltype(_proc(fwd<Args>(args)...))>>{}
        (_proc, *_mutLockable, fwd<Args>(args)...); // TODO: use source instead of * when available
    }
  };

  /// Helper function to deduce types for ScopeLockProc.
  ///
  /// Procedure<T (...)> Proc,
  /// Mutable<ScopeLockable> M
  template<typename Proc, typename M>
  ScopeLockProc<traits::Decay<Proc>, traits::Decay<M>> scopeLockProc(Proc&& proc, M&& mutLockable)
  {
    return { fwd<Proc>(proc), fwd<M>(mutLockable) };
  }

  /// A polymorphic transformation constructed from a ScopeLockable l that takes a procedure and
  /// returns an equivalent one that calls that procedure only if l was successfully locked and
  /// keeps the lock alive until the procedure returns.
  ///
  /// Example: mutex as a lockable to protect a asynchronous function call
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int globalCount = 0;
  /// void incrementGlobalCount()
  /// {
  ///   ++globalCount;
  /// }
  ///
  /// template<typename P>
  /// void asyncIncrementTenTimes(P syncTransfo)
  /// {
  ///   for (int i = 0; i < 10; ++i)
  ///     qi::async(syncTransfo(incrementGlobalCount));
  /// }
  ///
  /// void doStuff()
  /// {
  ///   static std::mutex m;
  ///   asyncIncrementTenTimes(scopeLockTransfo(&m));
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Mutable<ScopeLockable> M
  template<typename M>
  struct ScopeLockTransfo
  {
    M _mutLockable;
  // Regular (if members are regular):
    QI_GENERATE_FRIEND_REGULAR_OPS_1(ScopeLockTransfo, _mutLockable)
  // PolymorphicTransformation:
    /// Procedure<T (...)> Proc0
    template<typename Proc>
    ScopeLockProc<traits::Decay<Proc>, M> operator()(Proc&& p) const
    {
      return { fwd<Proc>(p), _mutLockable };
    }
  };

  /// Helper function to deduce types for ScopeLockTransfo.
  ///
  /// Mutable<ScopeLockable> M
  template<typename M>
  ScopeLockTransfo<traits::Decay<M>> scopeLockTransfo(M&& mutLockable)
  {
    return { fwd<M>(mutLockable) };
  }
} // namespace qi

#ifndef KA_CONCEPT_HPP
#define KA_CONCEPT_HPP
#pragma once

/// @file
/// Contain definitions of the core concepts used in `Ka`.

namespace ka {
/// # General definitions
///
/// A _concept_ is a conjunction of constraints on a type.
///
/// These constraints can be:
/// - _syntactic_ constraints
///     ex: the expression "++a" must be defined
/// - _semantic_ constraints
///     ex: "--(++a) == a, forall a"
/// - _time and space algorithmic complexity_ constraints
///     ex: "*a" must be O(1) in time
///
/// An algorithm is _generic_ if it specifies, in terms of concepts, the
/// minimal constraints for which it works.
///
/// A type that respects the constraints of a concept is said to _model_ this
/// concept.
///
/// Thus, the behavior of a generic algorithm is well defined for all its uses
/// with types modeling the required concepts.
///
/// When concepts are introduced as a C++ language feature, the concept
/// definitions below will be replaced by equivalent C++ code, reusing
/// concepts provided by the standard when possible.
///
///
/// # Syntax
///
/// As concepts are not yet part of the C++ language, they are specified here in
/// terms of structured comments.
/// The syntax used for their definition is as close as possible to the current
/// proposal (N4630). This way when concepts are available in the language, it
/// should be possible to make the switch with minimum effort.
///
/// Even in their current comment form, the user of these concepts is expected
/// to include this file to allow the reader an easy access to the definitions.
///
/// The syntax is (between angle brackets are the variable parts):
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   concept <concept_name>(<type_variable>) =
///        <constraint0>
///     && <constraint1>
///     && ...
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// ## Example:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   concept Iterator(I) =
///        Regular(I)
///     && ValueType: Iterator -> Regular
///     && ++: I& -> void
///     && *: I -> ValueType<I>
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This concept means that for a type I to be an Iterator:
/// - I must be Regular (this is another concept)
///
/// - A type traits named "ValueType" must be defined.
///   A type traits takes a type and "returns" another type at compile-time.
///   Here, it takes a type that is an Iterator and return a type that is Regular.
///   This means that anytime you have a type I that is an Iterator type, you can
///   write "ValueType<I>" and get the type pointed by iterators of type I.
///   For example, if I is an iterator on int, it is possible to write:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///     ValueType<I> n = 0;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   which is exactly the same as if you would have written directly:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///     int n = 0;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// - A "++" operator must be defined that changes the iterator in-place.
///   For example, with an instance "it" of an Iterator type, it is possible to
///   write:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///     ++it;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// - Finally, a "*" operation must be defined such that for example it is
///   possible to write:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///     ValueType<I> x = *it;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// ## Example:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   concept Semigroup(T) =
///        Regular(T)
///     && *: T x T -> T
///     && associative(*)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This concept means that for a type T to be a Semigroup:
/// - T must be Regular (this is another concept)
///
/// - T must have a function '*' defined, taking two instances of T and returning
///   an instance of T, such that you can write:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   T c = a * b;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// - '*' must be associative, that is:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///  (a * b) * c == a * (b * c) for all a, b, c values of T
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// ## Notes
///
/// Sometimes, a sentence or a real-usage example is clearer, so it is also
/// possible to use a less formal syntax:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetSslSocket(S, C, I) =
///      NetSslContext(C)
///   && NetIoService(I)
///   && HandshakeSide<S>: NetHandshakeSide
///   && Lowest<S>: NetLowestSocket
///   && With I ioServiceLValue,
///           C sslContext,
///           HandshakeSide<S> handshakeSide,
///           NetHandler handler, the following are valid:
///        S sslSocket{ioServiceLValue, sslContext};
///     && I& io = sslSocket.get_io_service();
///     && sslSocket.async_handshake(handshakeSide, handler)
///     && Lowest<S>& l = sslSocket.lowest_layer();
///     && auto& n = sslSocket.next_layer();
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// # Core concept definitions
///
/// ## QuasiRegular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept QuasiRegular(T) =
///     (forall a, b in T) T a = b; implies a == b
///  && (forall a, b in T) a = b implies a == b
///  && (forall a, b in T with a == b) modification on a implies a != b // copies independence
///  && equivalence(==)
///  && totalOrdering(<)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A QuasiRegular type is a Regular type without the default-constructibility.
/// See Regular.
///
///
/// ## Regular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Regular(T) =
///     QuasiRegular(T)
///  && T() yields an object in a partially-formed state.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// definition: partially-formed state
/// An object is in partially-formed state if it can only be assigned or destroyed.
/// Any other operation is undefined.
/// The main use is for default constructors. It allows them to possibly
/// do nothing, for example no allocation, and so on, and be later assigned.
/// It can be also useful for arrays, because they require default construction.
/// Also, moved-from objects are typically in this state.
///
/// Informally, a Regular type behaves like a builtin type (e.g int) with
/// respect to the construction, copy, assignment and equality.
/// This greatly simplifies value manipulation and makes easier algorithm
/// optimization by allowing substituting one expression by another
/// (simpler, optimized) equal one.
///
/// More specifically, this concept ensures for example that two copies,
/// either made via copy construction or assignment, are equal and independent.
/// Copy independence means that modifying an object doesn't affect its copies.
///
///
/// ## PseudoRegular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept PseudoRegular(T) =
///  Same as Regular except that the runtime constraints are dropped.
///  This means that all the expressions required by Regular must be
///  valid (i.e. must compile), but for example
///  T a = b; does not necessarily implies a == b anymore
///  (ditto for other implications).
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## Procedure
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Procedure(T, Args..., Ret) =
///      Regular(T)
///   && (): T& x Args... -> Ret
///   && () is not necessarily regular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A `Procedure` is a type you can use as a function. There is no guarantee that
/// calling several times an instance of a `Procedure` type will yield the same
/// result (this is what is meant by "not necessarily regular").
///
/// For example, with an instance "proc" of a
/// `Procedure(Proc, int, char, bool)` type, you can write:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   bool b0 = proc(5, 'z');
///   bool b1 = proc(5, 'z'); // here we might have `b1 != b0`
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Beware that even if the *procedure call* is not necessarily regular,
/// a *`Procedure` type* itself is `Regular`, for example:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   auto p0 = proc; // invariant: `p0 == proc`
///   bool p1 = p0;   // invariant: `p1 == p0 && p1 == proc`
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## Function
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Function(F, Args..., Ret) =
///      Procedure(F, Args..., Ret)
///   && () is regular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This is a mathematical function. Calling it with equal inputs is guaranteed
/// to yield equal outputs.
///
/// For example, with an instance "f" of a
/// `Function(F, int, char, bool)` type, you can write:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///   bool b0 = f(5, 'z');
///   bool b1 = f(5, 'z'); // invariant: `b1 == b0`
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## Transformation
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Transformation(F, T) =
///      Function(F, T, T)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A transformation is a `Function` taking a single argument and
/// returning a value of the same type.
///
/// For example, "char toUpper(char)" is a transformation.
///
/// The notation "f^n" means that f is a transformation that is called n times.
///
/// For example, with an instance f of a Transformation type:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// f^3(x) means f(f(f(x))).
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// By definition,
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// f^0(x) == x.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## Accumulation
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Accumulation(F, T, Args...) =
///      Function(F, T&, Args..., void)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// An accumulation stores its result in a argument passed by reference instead
/// of returning it. By avoiding creating a new value to return, an `Accumulation`
/// may be more efficient than the corresponding `Function`.
/// `Accumulation(F, T, Args...)` is semantically equivalent to
/// `Function(F, Args..., T)`, that is the accumulation signature `void (T&, Args...)`
/// is semantically equivalent to `T (Args...)`.
///
/// For example, below `concat` is an instance of
/// `Accumulation(std::string, const std::string&, int)`:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// auto concat = [](std::string& s0, std::string const& s1, int n) {
///   // Precondition: n >= 0
///   while (n > 0) {
///     s0 += s1;
///     --n;
///   }
///   // no returned value
/// };
/// std::string s{"abc"};
/// concat(s, "def", 3); // s == "abcdefdefdef"
/// concat(s, "\/?", 4); // s == "abcdefdefdef\/\/\/\/"
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## Action
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Action(F, T) =
///      Accumulation(F, T)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// An action is an accumulation on a single argument.
/// `Action(F, T)` is thus semantically equivalent to `Transformation(F, T)`, that
/// is the action signature `void (T&)` is semantically equivalent to the
/// transformation signature `T (T)`.
///
/// For example, if "popping" a range means "removing its first element",
/// you could write a transformation
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///  popTransfo: Rng -> Rng
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// returning a new range without the first element and not modifying the
/// original range,
/// which would be equivalent to the action
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///  popAction: Rng& -> void
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// modifying the given range directly.
///
/// The benefit of an action is to avoid a potentially expensive copy, with
/// the drawback of a somewhat more cumbersome writing.
///
/// For a real example, the Range concept defines "pop" as an action.
///
/// For convenience, this notation used for powers of transformation is extended
/// to actions.
/// For example, with an instance `a` of an Action type:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// a^3(x) means (a(x), a(x), a(x)) (i.e. three successive calls)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## IsomorphicAction
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept IsomorphicAction(F, A) =
///      Action(F, A)
///   && retract: F -> G where:
///         Action(G, A)
///      && With g == retract(f):
///            (forall x in Arg) (f(x), g(x)) == x
///         && (forall x in Arg) (g(x), f(x)) == x
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This is an action that can be retracted, i.e. undone, in both directions.
/// Another way to put it is that it is invertible.
///
///
/// ## RetractableFunction
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept RetractableFunction(F, Arg, Ret) =
///      Function(F, Arg, Ret)
///   && retract: F -> G where:
///         Function(G, Ret, Arg)
///      && With g == retract(f), the following is valid:
///        (forall x in Arg) g(f(x)) == x
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This is a mathematical unary function that can be retracted, i.e. undone.
///
/// Warning: That does not mean that the other way can be undone, i.e.
///     (forall y in Ret) f(retract(f)(y)) == y
///   is false.
///
///
/// ## Isomorphism
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Isomorphism(F, Arg, Ret) =
///      RetractableFunction(F, Arg, Ret)
///   && With F f,
///           g == retract(f), the following is valid:
///     (forall x in Ret) f(g(x)) == x
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This is an invertible function, i.e. it can be retracted (undone) in both
/// directions.
/// If `f` is an isomorphism and `g` its retraction, between types `X` and `Y`,
/// such as
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///      f
///   X ---> Y
///     <---
///      g
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// the following is true:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///      g(f(x)) == x, for all x in X
///   && f(g(y)) == y, for all y in Y
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This is in contrast to a `RetractableFunction` where only `g(f(x)) == x` is
/// true.
///
///
/// ## Readable
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Readable(T) =
///      Regular(T)
///   && With T t, the following is valid:
///     auto const& value = src(t);
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A Readable is a type that gives access to an underlying value. It is only
/// allowed to read the underlying value, not modifying it.
/// Typical models are: pointers const, iterators const, boost::optional const,
/// pointers, iterators, boost::optional
///
/// Note: A type modeling `Readable` can directly implement `src` or can provide
///   `operator*` and rely on the global `src` for it to be called.
///
///
/// ## Mutable
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Mutable(T) =
///      Regular(T)
///   && Readable(T)
///   && With T const t:
///     `src(t) = x` is well-formed and establishes `src(t) == x`
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A Mutable is a type that you can dereference. The dereferenced value can be
/// modified.
/// Typical models are: pointers, iterators, boost::optional
/// Note: Constness of a value of a Mutable type does not imply constness of the
///   referenced value (same behavior as native pointers).
///
///
/// ## Tuple
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Tuple(T) =
///      Regular(T)
///   && With T t, constexpr std::size_t I, the following is valid:
///        constexpr std::size_t N = std::tuple_size<T>::value;
///     && auto& x = std::get<I>(t); // for I in [0, N)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A Tuple is a heterogeneous container that can be unpacked.
/// See `apply` for an example of unpacking.
/// Typical models are: std::tuple, std::pair, std::array
/// Note: Any user-defined type can be made to model this concept.
///
///
/// ## ScopeLockable
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept ScopeLockable(T) =
///   With T& t, the following is valid:
///     {
///       if (auto lock = scopelock(t)) {
///         // In this scope, t is locked.
///       }
///     }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// A ScopeLockable ensures that a guarantee is true during the lifetime of the object acquired
/// from the call to the scopelock function (often called  a "lock", as in the example).
/// The guarantee could be, for example, a safe mutually exclusive access to some data, or an
/// acquisition and release of a resource, or another similarly symetric mechanism.
/// Typical models are: std::mutex, std::recursive_mutex, std::weak_ptr<T>, std::atomic_flag,
/// boost::synchronized_value<T>.
///
///
/// # Range concept definitions
///
/// ## Range
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Range(R) =
///     Regular(R)
///  && is_empty: R -> bool
///  && pop: R -> void
///  && pop is not necessarily regular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// In this basic concept, you can "iterate" through the range but not
/// access the values. Which can be useful if you're only interested in
/// advancing the front of the range.
///
/// The typical use is:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// while (!is_empty(myRange)) {
///   <some code>
///   pop(myRange);
/// }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Note that is_empty is not guaranteed to ever return false, i.e. the
/// range could be infinite.
/// Also, pop being not necessarily regular, the traversal on a copy of a range
/// is not guaranteed to yield the same result (e.g. useful for input streams).
///
///
/// ## ForwardRange
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept ForwardRange(R) =
///     Range(R)
///  && pop is regular (i.e. the range is multipass)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## ReadableRange
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept ReadableRange(R) =
///     Range(R)
///  && front: R -> U where Regular(U)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## ReadableForwardRange
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept ReadableForwardRange(R) =
///     ForwardRange(R)
///  && ReadableRange(R)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## MutableForwardRange
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept MutableForwardRange(R) =
///     ReadableForwardRange(R)
///  && (forall r in R where front(r) is defined) front(r) = x establishes front(r) == x
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## Empty
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Empty(T) =
///     Regular(T)
///  && ka::empty: T -> bool
///  && (forall t in T) !ka::empty(t) implies regular operations are defined
///       (copy, assignment, equality, etc.)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
///
/// ## EmptyProcedure
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept EmptyProcedure(T) =
///     Empty(T)
///  && Procedure(T)
///  && (forall t in T) !ka::empty(t) implies procedure call is defined
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Typical models are: `std::function`, `boost::function`
///
///
/// ## EmptyMutable
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept EmptyMutable(T) =
///     Empty(T)
///  && Mutable(T)
///  && (forall t in T) !ka::empty(t) implies mutable operations are defined
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Typical models are: pointers, `std::shared_ptr`, `boost::shared_ptr`
///
///
/// ## Functor
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Functor(T) =
///     Regular(T<A>)
///  && fmap: F x T<A> -> T<B>, where Function<B (A)> F
///  && With `g ∘ f` denoting `compose(g, f)`,
///          `fmap(h)` denoting partial function application of `fmap`,
///          id_transfo_t id,
///          `==` denoting function extensional equality (i.e. two functions are
///          equal if for all inputs they have the same output):
///       fmap(id) == id                   (preservation of identity)
///    && fmap(g ∘ f) == fmap(g) ∘ fmap(f) (preservation of composition)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A functor is a projection of objects and morphisms (here resp., types
/// and functions) that preserves identity and morphism composition (see
/// contraints on `fmap` above).
///
/// The `Functor` concept implements the type projection through a template type
/// (`T` above), and the function projection (also called sometimes 'lifting')
/// through the `fmap` function.
///
/// Note: Projected functions are unary. See concept `FunctorApp` for n-ary
/// functions.
///
/// The functor notion notably allows one to dissociate the work to apply to a
/// value placed inside a data structure (the "what"), from how to actually
/// apply it inside the data structure (the "how"). But beware that not all
/// functor models are container-like, such as functions with a given domain, or
/// futures.
///
/// Typical models are: std containers, std::optional, functions with a given
/// domain, futures, etc.
///
///
/// ## FunctorApp
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept FunctorApp(T) =
///     Functor(T)
///  && fmap: F x T<A> x ... -> T<Z>, where Function<Z (A x ...)> F
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Functor that allows one to apply a n-ary function. The name is reminiscent
/// of the notion of `Applicative` functor found in other languages.
///
/// Typical models are: non-key/value std containers (e.g. this includes
/// `std::set` but excludes `std::map`), std::optional, futures, etc.

namespace concept { // To allow doc tools to extract this documentation.
}
} // namespace ka

#endif // KA_CONCEPT_HPP

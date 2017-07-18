#ifndef _QI_CONCEPT_HPP_
#define _QI_CONCEPT_HPP_

/// @file
/// Contain definitions of the core concepts used in libqi.

namespace qi
{
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
/// A Procedure is a type you can use as a function. There is no guarantee that
/// calling several times an instance of a Procedure type will yield the same
/// result (this is what is meant by "not necessarily regular").
/// For example, with an instance "proc" of a
/// Procedure(Proc, int, char, bool) type, you can write:
///   bool b = proc(5, 'z');
///
///
/// ## Transformation
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Transformation(F, A) =
///      Procedure(F, A, A)
///   && () is regular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A transformation is a regular function taking a single argument and
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
/// ## Action
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Action(F, A) =
///      Procedure(F, A&, void)
///   && () is regular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// An action is a regular function taking a value by reference and returning
/// nothing.
/// It is semantically equivalent to a transformation on T.
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
/// For example, with an instance "a" of an Action type:
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
/// ## Function
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Function(F, Args..., Ret) =
///      Procedure(F, Args..., Ret)
///   && () is regular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This is a mathematical function.
///
///
/// ## RetractableFunction
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept RetractableFunction(F, Arg, Ret) =
///      Function(F, Arg, Ret)
///   && retract: F -> G where:
///         Function(G, Ret, Arg)
///      && (forall x in Arg) retract(f)(f(x)) == x
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// This is a mathematical unary function that can be retracted, i.e. undone.
///
/// Warning: That does not mean that the other way can be undone, i.e.
///     (forall y in Ret) f(retract(f)(y)) == y
///   is false.
///
///
/// ## Readable
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Readable(T) =
///      Regular(T)
///   && With T t, the following is valid:
///     const auto& value = *t;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A Readable is a type that you can dereference. It is only allowed to read
/// the dereferenced value, not modifying it.
/// Typical models are: const pointers, const iterators, const boost::optional,
/// pointers, iterators, boost::optional
///
///
/// ## Mutable
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Mutable(T) =
///      Regular(T)
///   && Readable(T)
///   && With T t:
///     `*t = x` is well-formed and establishes `*t == x`
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A Mutable is a type that you can dereference. The dereferenced value can be
/// modified.
/// Typical models are: pointers, iterators, boost::optional
///
///
/// # Range concept definitions
///
/// ## Range
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Range(R) =
///     Regular(R)
///  && isEmpty: R -> bool
///  && pop: R -> void
///  && pop is not necessarily regular
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// In this basic concept, you can "iterate" through the range but not
/// access the values. Which can be useful if you're only interested in
/// advancing the front of the range.
///
/// The typical use is:
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// while (!isEmpty(myRange)) {
///   <some code>
///   pop(myRange);
/// }
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Note that isEmpty is not guaranteed to ever return false, i.e. the
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
namespace concept // To allow doc tools to extract this documentation.
{
}
} // namespace qi

#endif // _QI_CONCEPT_HPP_

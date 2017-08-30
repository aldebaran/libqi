#pragma once
#include <tuple>
#include <utility>
#include <boost/config.hpp>
#include <qi/type/traits.hpp>
#include <qi/utility.hpp>
#include <qi/macroregular.hpp>

namespace qi
{
  namespace detail
  {
    // Extracts the value, if the tuple has a size of 1.
    // Otherwise, return the tuple as-is.
    template<typename T>
    T& unpackSingle(std::tuple<T>& t)
    {
      return std::get<0>(t);
    }

    inline std::tuple<>& unpackSingle(std::tuple<>& t)
    {
      return t;
    }

    template<typename T, typename U, typename... V>
    std::tuple<T, U, V...>& unpackSingle(std::tuple<T, U, V...>& t)
    {
      return t;
    }
  } // namespace detail

  /// Moves its value on copy.
  ///
  /// This means that the copy constructor will move the members instead of
  /// copying them.
  /// This is useful in C++11 to simulate a capture by move in a lambda.
  ///
  /// Example: Moving a single value inside a lambda
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // 0) `sslContext` is move-only and we want to move it inside a lambda for a
  /// // delayed use.
  ///
  /// // 1) First, move it into a `MoveOnCopy` object.
  /// auto moc = makeMoveOnCopy(std::move(sslContext));
  ///
  /// // 2) Then, capture the `MoveOnCopy` object by value in the lambda.
  /// // Since `MoveOnCopy`'s copy constructor in fact moves its members,
  /// // `sslContext` is moved and not copied.
  ///
  /// async_connect([=](ErrorCode e) { // captures `context` by value
  ///
  ///   // 3) Unpack the ssl context and move it to the next procedure.
  ///   auto s = createSocket(std::move(*moc));
  ///   // ...
  /// });
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// It is also possible to capture several elements. In this case a `std::tuple`
  /// is returned on dereferencing and `apply` can be used to unpack the arguments.
  ///
  /// Example: Forwarding several arguments inside a lambda and passing them to a procedure.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// template<typename Proc, typename... Args>
  /// void post(Proc proc, Args&&... args)
  /// {
  ///   // 1) Move all arguments inside a `MoveOnCopy`.
  ///   auto moc = makeMoveOnCopy(fwd<Args>(args)...);
  ///   async([=] {
  ///
  ///     // 2) Call the procedure (`apply` automatically unpacks the tuple in n arguments).
  ///     apply(proc, std::move(asTuple(moc))); // See note below.
  ///   });
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Note: Internally, values passed to `MoveOnCopy` are stored in a tuple.
  /// `operator*` returns this tuple, except when the tuple contains only one value.
  /// In this case, `operator*` returns the value directly (and not the tuple).
  /// This is because storing a single value is the most common case, and this
  /// behavior is the most intuitive.
  /// But in a generic context with variadic methods, this non-homogeneous
  /// behavior is problematic. Therefore in a generic context, you should use
  /// `asTuple` instead of `operator*` as it will return a tuple, even if it
  /// contains a single value. This allows for example the use of `apply` without
  /// further complication.
  ///
  /// Warning: Because of the move, the source operand of a copy is not valid
  ///   anymore after the copy.
  template<typename... T>
  class MoveOnCopy
  {
    using tuple_type = std::tuple<T...>;
    tuple_type values;
  public:
    MoveOnCopy() = default;

    template<typename U, typename = traits::EnableIfNotBaseOf<MoveOnCopy, U>>
    explicit MoveOnCopy(U&& u)
      : values(std::forward<U>(u))
    {
    }

    template<typename U, typename V, typename... W>
    explicit MoveOnCopy(U&& u, V&& v, W&&... w)
      : values(std::forward<U>(u), std::forward<V>(v), std::forward<W>(w)...)
    {
    }

    MoveOnCopy(const MoveOnCopy& x)
      : values(std::move(asTuple(x)))
    {
    }

    // TODO: Use default version when get rid of VS2013.
    MoveOnCopy(MoveOnCopy&& x) BOOST_NOEXCEPT
      : values(std::move(x.values))
    {
    }

    MoveOnCopy& operator=(const MoveOnCopy& x)
    {
      values = std::move(asTuple(x));
      return *this;
    }

    // TODO: Use default version when get rid of VS2013.
    MoveOnCopy& operator=(MoveOnCopy&& x) BOOST_NOEXCEPT
    {
      values = std::move(x.values);
    }

    // MoveOnCopy is not regular but we can still provide the relational operators.
    QI_GENERATE_FRIEND_REGULAR_OPS_1(MoveOnCopy, values)

  // Custom:
    /// Returns stored values as a `std::tuple`.
    friend tuple_type& asTuple(const MoveOnCopy& x)
    {
      return const_cast<tuple_type&>(x.values);
    }

  // Mutable<T>:
    /// Returns the stored values as a `std::tuple` if there are 0 or n > 1 values.
    /// Otherwise (only 1 value), returns this value directly (not a tuple).
    ///
    /// Note: As a Mutable, constness of the `MoveOnCopy` does not imply
    /// constness of the "indirected" object.
    auto operator*() const -> decltype(detail::unpackSingle(declref<tuple_type>()))
    {
      return detail::unpackSingle(asTuple(*this));
    }
  };

  /// Helper function to perform type deduction for constructing a MoveOnCopy.
  template<typename... T>
  MoveOnCopy<traits::Decay<T>...> makeMoveOnCopy(T&&... t)
  {
    return MoveOnCopy<traits::Decay<T>...>{std::forward<T>(t)...};
  }
} // namespace qi

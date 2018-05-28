#ifndef KA_MOVEONCOPY_HPP
#define KA_MOVEONCOPY_HPP
#pragma once
#include <tuple>
#include <utility>
#include <boost/config.hpp>
#include "macroregular.hpp"
#include "typetraits.hpp"
#include "utility.hpp"

namespace ka {
  namespace detail {
    // Extracts the value, if the tuple has a size of 1.
    // Otherwise, return the tuple as-is.
    template<typename T>
    T& unpack_single(std::tuple<T>& t) {
      return std::get<0>(t);
    }

    inline std::tuple<>& unpack_single(std::tuple<>& t) {
      return t;
    }

    template<typename T, typename U, typename... V>
    std::tuple<T, U, V...>& unpack_single(std::tuple<T, U, V...>& t) {
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
  /// // 0) `ssl_context` is move-only and we want to move it inside a lambda for a
  /// // delayed use.
  ///
  /// // 1) First, move it into a `move_on_copy_t` object.
  /// auto moc = move_on_copy(std::move(ssl_context));
  ///
  /// // 2) Then, capture the `move_on_copy_t` object by value in the lambda.
  /// // Since `move_on_copy_t`'s copy constructor in fact moves its members,
  /// // `ssl_context` is moved and not copied.
  ///
  /// async_connect([=](ErrorCode e) { // captures `context` by value
  ///
  ///   // 3) Unpack the ssl context and move it to the next procedure.
  ///   auto s = create_socket(std::move(*moc));
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
  /// void post(Proc proc, Args&&... args) {
  ///   // 1) Move all arguments inside a `move_on_copy_t`.
  ///   auto moc = move_on_copy(fwd<Args>(args)...);
  ///   async([=] {
  ///
  ///     // 2) Call the procedure (`apply` automatically unpacks the tuple in n arguments).
  ///     apply(proc, std::move(as_tuple(moc))); // See note below.
  ///   });
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Note: Internally, values passed to `move_on_copy_t` are stored in a tuple.
  /// `operator*` returns this tuple, except when the tuple contains only one value.
  /// In this case, `operator*` returns the value directly (and not the tuple).
  /// This is because storing a single value is the most common case, and this
  /// behavior is the most intuitive.
  /// But in a generic context with variadic methods, this non-homogeneous
  /// behavior is problematic. Therefore in a generic context, you should use
  /// `as_tuple` instead of `operator*` as it will return a tuple, even if it
  /// contains a single value. This allows for example the use of `apply` without
  /// further complication.
  ///
  /// Warning: Because of the move, the source operand of a copy is not valid
  ///   anymore after the copy.
  template<typename... T>
  class move_on_copy_t {
    using tuple_type = std::tuple<T...>;
    tuple_type values;
  public:
    move_on_copy_t() = default;

    template<typename U, typename = EnableIfNotBaseOf<move_on_copy_t, U>>
    explicit move_on_copy_t(U&& u)
      : values(fwd<U>(u)) {
    }

    template<typename U, typename V, typename... W>
    explicit move_on_copy_t(U&& u, V&& v, W&&... w)
      : values(fwd<U>(u), fwd<V>(v), fwd<W>(w)...) {
    }

    move_on_copy_t(move_on_copy_t const& x)
      : values(std::move(as_tuple(x))) {
    }

    // TODO: Use default version when get rid of VS2013.
    move_on_copy_t(move_on_copy_t&& x) BOOST_NOEXCEPT
      : values(std::move(x.values)) {
    }

    move_on_copy_t& operator=(move_on_copy_t const& x) {
      values = std::move(as_tuple(x));
      return *this;
    }

    // TODO: Use default version when get rid of VS2013.
    move_on_copy_t& operator=(move_on_copy_t&& x) BOOST_NOEXCEPT {
      values = std::move(x.values);
    }

    // move_on_copy_t is not regular but we can still provide the relational operators.
    KA_GENERATE_FRIEND_REGULAR_OPS_1(move_on_copy_t, values)

  // Custom:
    /// Returns stored values as a `std::tuple`.
    friend tuple_type& as_tuple(move_on_copy_t const& x) {
      return const_cast<tuple_type&>(x.values);
    }

  // Mutable<T>:
    /// Returns the stored values as a `std::tuple` if there are 0 or n > 1 values.
    /// Otherwise (only 1 value), returns this value directly (not a tuple).
    ///
    /// Note: As a Mutable, constness of the `move_on_copy_t` does not imply
    /// constness of the "indirected" object.
    auto operator*() const -> decltype(detail::unpack_single(declref<tuple_type>())) {
      return detail::unpack_single(as_tuple(*this));
    }
  };

  /// Helper function to perform type deduction for constructing a move_on_copy_t.
  template<typename... T>
  move_on_copy_t<Decay<T>...> move_on_copy(T&&... t) {
    return move_on_copy_t<Decay<T>...>{fwd<T>(t)...};
  }
} // namespace ka

#endif // KA_MOVEONCOPY_HPP

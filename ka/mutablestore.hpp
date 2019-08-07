#ifndef KA_MUTABLESTORE_HPP
#define KA_MUTABLESTORE_HPP
#pragma once
#include <functional>
#include <boost/variant.hpp>
#include "macroregular.hpp"
#include "typetraits.hpp"
#include "utility.hpp"

namespace ka {
  /// Dereferenceable type that contains a value or an indirection to an object.
  ///
  /// Here the "indirection type" models the Mutable concept (see concept.hpp).
  /// It means that it can be dereferenced and the dereferenced value can be
  /// modified. If not specified, the "indirection type" is a pointer type
  /// (i.e. if the value type is `T`, it will be by default `T*`).
  ///
  /// Useful if you want to act on a resource and knowing if it has a value
  /// semantics or an entity semantics is irrelevant.
  ///
  /// This type itself models the concept Mutable (see concept.hpp).
  /// This means that it can be dereferenced and the dereferenced value can be
  /// modified.
  ///
  /// On dereferencing:
  /// - if the instance directly contains the value, a reference on it is returned.
  /// - if the instance contains an indirection to an object, the result of the
  ///   dereferencing is returned.
  ///
  /// Example: Storing an integer or a pointer to an integer
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct X {
  ///   mutable_store_t<int, int*> counter; // owns an `int` or an `int*`.
  ///
  ///   void increment() {
  ///     // Modifies the integer in `counter` or the integer pointed to by `counter`.
  ///     // In the latter case, it is the user's responsibility to ensure there
  ///     // is no data race.
  ///     ++(*counter);
  ///   }
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// Note: Typical "indirection types" for a value type T are:
  ///   T*, std::unique_ptr<T>, std::shared_ptr<T>, ka::reference_wrapper<T>
  ///
  /// Note: If you use a move-only "indirection type" such as `std::unique_ptr`
  ///   the resulting `mutable_store_t` will also be move-only.
  ///
  /// Example: Using the same ssl context for all accepted sockets (server side)
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct socket_with_context_t {
  ///   socket_t socket;
  ///   mutable_store_t<ssl_context> context; // Contains a ssl_context or a ssl_context*.
  ///   // ...
  /// };
  ///
  /// // `accepted_socket` has type `socket_t`.
  /// // Here, the server maintains the ssl context and shares it between all
  /// // accepted socket instances.
  /// auto socket_ptr = boost::make_shared<socket_with_context_t>(accepted_socket, &sslContext);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Creating a new ssl context for each created socket (client side)
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct message_socket {
  ///   socket_with_context_t socket; // see previous example
  ///   // other members...
  ///
  ///   message_socket()
  ///     : socket(socket_t{}, ssl_context{ssl_context::v23})
  ///     // ...
  ///   {
  ///     // The ssl context is moved inside the `mutable_store_t` of `socket_with_context_t`.
  ///   }
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Mutable<T> M
  template<typename T, typename M = T*>
  class mutable_store_t {
    using variant_type = boost::variant<M, T>;
    variant_type data;

    struct deref : boost::static_visitor<T&> {
      T& operator()(T& x) const KA_NOEXCEPT(true) {
        return x;
      }
      T& operator()(M& x) const KA_NOEXCEPT_EXPR(*x) {
        return *x;
      }
    };
  public:
    /// With U&& u, the following is valid:
    ///   boost::variant<M, T>(fwd<U>(u))
    template<typename U, typename = EnableIfNotBaseOf<mutable_store_t, U>>
    explicit mutable_store_t(U&& u) KA_NOEXCEPT_EXPR(variant_type(fwd<U>(u)))
        : data(fwd<U>(u)) {
    }

    /// With boost::variant<M, T> v, U&& u, the following is valid:
    ///   v = fwd<U>(u);
    template<typename U, typename = EnableIfNotBaseOf<mutable_store_t, U>>
    mutable_store_t& operator=(U&& u) KA_NOEXCEPT_EXPR(data = fwd<U>(u)) {
      data = fwd<U>(u);
      return *this;
    }
  // Regular (if T is Regular):
    mutable_store_t() = default;

    // TODO: Use default version when get rid of VS2013.
    mutable_store_t(mutable_store_t const& x) KA_NOEXCEPT_EXPR(variant_type(variant_type()))
      : data(x.data) {
    }

    // TODO: Use default version when get rid of VS2013.
    mutable_store_t& operator=(mutable_store_t const& x) KA_NOEXCEPT_EXPR(data = x.data) {
      data = x.data;
      return *this;
    }

    // TODO: Use default version when get rid of VS2013.
    mutable_store_t(mutable_store_t&& x) KA_NOEXCEPT(true)
      : data(std::move(x.data)) {
    }

    // TODO: Use default version when get rid of VS2013.
    mutable_store_t& operator=(mutable_store_t&& x) KA_NOEXCEPT(true) {
      data = std::move(x.data);
      return *this;
    }

    KA_GENERATE_FRIEND_REGULAR_OPS_1(mutable_store_t, data)

  // Mutable<T>:
    /// Returns a non-const reference on the owned object or on the dereferenced
    /// object.
    ///
    /// Note: The constness of a mutable object does not imply the constness of
    ///   the dereferenced object (same behavior as native pointers).
    T& operator*() const KA_NOEXCEPT_EXPR(boost::apply_visitor(deref{}, const_cast<variant_type&>(data))) {
      return boost::apply_visitor(deref{}, const_cast<variant_type&>(data));
    }
  };

  namespace detail {
    template<typename T>
    mutable_store_t<Decay<T>, Decay<T>*> mutable_store_fwd_impl(T&& t, std::true_type /*isRValue*/) {
      // rvalue: move it inside the mutable.
      return mutable_store_t<Decay<T>, Decay<T>*>{std::move(t)};
    }

    template<typename T>
    mutable_store_t<Decay<T>, Decay<T>*> mutable_store_fwd_impl(T&& t, std::false_type /*isRValue*/) {
      // Not an rvalue (i.e it is an lvalue): put the address inside the mutable.
      return mutable_store_t<Decay<T>, Decay<T>*>{&t};
    }

    template<typename T>
    using Raw = RemovePointer<Decay<T>>;
  } // namespace detail

  /// Helper function to perform type-deduction for `mutable_store_t`.
  ///
  /// Type deduction proceeds this way:
  ///
  /// - if a pointer of type `T*` is passed, the returned
  ///   type is `mutable_store_t<T, T*>` containing the given pointer.
  ///
  /// - otherwise, a non-pointer object of type `T` is passed, and the returned
  ///   type is `mutable_store_t<T, T*>` containing the given object.
  ///
  /// Example: Storing a value by address
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int j = 0;
  /// auto m = mutable_store(&j);
  /// ++(*m);
  /// // here, `j == 1`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Storing a value by value
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int j = 0;
  /// auto m = mutable_store(j); // copy the value
  /// ++(*m);
  /// // here, `j == 0`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template<typename T>
  mutable_store_t<detail::Raw<T>, detail::Raw<T>*> mutable_store(T&& t) {
    using R = detail::Raw<T>;
    return mutable_store_t<R, R*>{fwd<T>(t)};
  }

  /// Helper function to perform delayed perfect-forwarding in a generic context.
  ///
  /// Example: Perfect-forwarding a parameter after capture in a lambda
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// template<typename T>
  /// void f(T&& t) {
  ///   // 1) Store `t` by address if it is an L-value, by value otherwise.
  ///   auto mut = mutable_store_fwd(fwd<T>(t));
  ///
  ///   // 2) Wrap `mut` to move it into the lambda.
  ///   auto moc = move_on_copy(mut);
  ///
  ///   async([=] {
  ///
  ///     // 3) Get the value back and perfect-forward it to the next procedure.
  ///     //    `**moc` is a L-value ref so `fwd<T>(**moc)` works as expected.
  ///     g(fwd<T>(**moc));
  ///   });
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// If you just want to construct a `mutable_store_t` with type deduction, simply
  /// use `makemutable_store_t`.
  template<typename T>
  mutable_store_t<Decay<T>, Decay<T>*> mutable_store_fwd(T&& t) {
    return detail::mutable_store_fwd_impl(std::forward<T>(t), std::is_rvalue_reference<T&&>{});
  }

} // namespace ka

#endif // KA_MUTABLESTORE_HPP

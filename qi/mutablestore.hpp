#pragma once
#include <functional>
#include <boost/config/suffix.hpp>
#include <boost/variant.hpp>
#include <qi/type/traits.hpp>
#include <qi/utility.hpp>
#include <qi/macroregular.hpp>

namespace qi {
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
  ///   MutableStore<int, int*> counter; // owns an `int` or an `int*`.
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
  ///   T*, std::unique_tr<T>, std::shared_ptr<T>, qi::ReferenceWrapper<T>
  ///
  /// Note: If you use a move-only "indirection type" such as `std::unique_ptr`
  ///   the resulting `MutableStore` will also be move-only.
  ///
  /// Example: Using the same ssl context for all accepted sockets (server side)
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct SocketWithContext
  /// {
  ///   Socket socket;
  ///   MutableStore<SslContext> context; // Contains a SslContext or a SslContext*.
  ///   // ...
  /// };
  ///
  /// // `acceptedSocket` has type `Socket`.
  /// // Here, the server maintains the ssl context and shares it between all
  /// // accepted socket instances.
  /// auto socketPtr = boost::make_shared<SocketWithContext>(acceptedSocket, &sslContext);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Creating a new ssl context for each created socket (client side)
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct MessageSocket
  /// {
  ///   SocketWithContext socket; // see previous example
  ///   // other members...
  ///
  ///   MessageSocket()
  ///     : socket(Socket{}, SslContext{SslContext::v23})
  ///     // ...
  ///   {
  ///     // The ssl context is moved inside the `MutableStore` of `SocketWithContext`.
  ///   }
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Mutable<T> M
  template<typename T, typename M = T*>
  class MutableStore
  {
    using variant_type = boost::variant<M, T>;
    variant_type data;

    struct Deref : boost::static_visitor<T&>
    {
      T& operator()(T& x) const QI_NOEXCEPT(true)
      {
        return x;
      }
      T& operator()(M& x) const QI_NOEXCEPT_EXPR(*x)
      {
        return *x;
      }
    };
  public:
    /// With U&& u, the following is valid:
    ///   boost::variant<M, T>(fwd<U>(u))
    template<typename U, typename = traits::EnableIfNotBaseOf<MutableStore, U>>
    explicit MutableStore(U&& u) QI_NOEXCEPT_EXPR(variant_type(fwd<U>(u)))
        : data(fwd<U>(u))
    {
    }

    /// With boost::variant<M, T> v, U&& u, the following is valid:
    ///   v = fwd<U>(u);
    template<typename U, typename = traits::EnableIfNotBaseOf<MutableStore, U>>
    MutableStore& operator=(U&& u) QI_NOEXCEPT_EXPR(data = fwd<U>(u))
    {
      data = fwd<U>(u);
      return *this;
    }
  // Regular (if T is Regular):
    MutableStore() = default;

    // TODO: Use default version when get rid of VS2013.
    MutableStore(const MutableStore& x) QI_NOEXCEPT_EXPR(variant_type(variant_type()))
      : data(x.data)
    {
    }

    // TODO: Use default version when get rid of VS2013.
    MutableStore& operator=(const MutableStore& x) QI_NOEXCEPT_EXPR(data = x.data)
    {
      data = x.data;
      return *this;
    }

    // TODO: Use default version when get rid of VS2013.
    MutableStore(MutableStore&& x) QI_NOEXCEPT(true)
      : data(std::move(x.data))
    {
    }

    // TODO: Use default version when get rid of VS2013.
    MutableStore& operator=(MutableStore&& x) QI_NOEXCEPT(true)
    {
      data = std::move(x.data);
      return *this;
    }

    QI_GENERATE_FRIEND_REGULAR_OPS_1(MutableStore, data)

  // Mutable<T>:
    /// Returns a non-const reference on the owned object or on the dereferenced
    /// object.
    ///
    /// Note: The constness of a mutable object does not imply the constness of
    ///   the dereferenced object (same behavior as native pointers).
    T& operator*() const QI_NOEXCEPT_EXPR(boost::apply_visitor(Deref{}, const_cast<variant_type&>(data)))
    {
      return boost::apply_visitor(Deref{}, const_cast<variant_type&>(data));
    }
  };

  namespace detail
  {
    template<typename T>
    MutableStore<traits::Decay<T>, traits::Decay<T>*> makeMutableStoreFwdImpl(T&& t, std::true_type /*isRValue*/)
    {
      // rvalue: move it inside the mutable.
      return MutableStore<traits::Decay<T>, traits::Decay<T>*>{std::move(t)};
    }

    template<typename T>
    MutableStore<traits::Decay<T>, traits::Decay<T>*> makeMutableStoreFwdImpl(T&& t, std::false_type /*isRValue*/)
    {
      // Not an rvalue (i.e it is an lvalue): put the address inside the mutable.
      return MutableStore<traits::Decay<T>, traits::Decay<T>*>{&t};
    }

    template<typename T>
    using Raw = traits::RemovePointer<traits::Decay<T>>;
  } // namespace detail

  /// Helper function to perform type-deduction for `MutableStore`.
  ///
  /// Type deduction proceeds this way:
  ///
  /// - if a pointer of type `T*` is passed, the returned
  ///   type is `MutableStore<T, T*>` containing the given pointer.
  ///
  /// - otherwise, a non-pointer object of type `T` is passed, and the returned
  ///   type is `MutableStore<T, T*>` containing the given object.
  ///
  /// Example: Storing a value by address
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int j = 0;
  /// auto m = makeMutableStore(&j);
  /// ++(*m);
  /// // here, `j == 1`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Storing a value by value
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int j = 0;
  /// auto m = makeMutableStore(j); // copy the value
  /// ++(*m);
  /// // here, `j == 0`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template<typename T>
  MutableStore<detail::Raw<T>, detail::Raw<T>*> makeMutableStore(T&& t)
  {
    using R = detail::Raw<T>;
    return MutableStore<R, R*>{fwd<T>(t)};
  }

  /// Helper function to perform delayed perfect-forwarding in a generic context.
  ///
  /// Example: Perfect-forwarding a parameter after capture in a lambda
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// template<typename T>
  /// void f(T&& t) {
  ///   // 1) Store `t` by address if it is an L-value, by value otherwise.
  ///   auto mut = makeMutableStoreFwd(fwd<T>(t));
  ///
  ///   // 2) Wrap `mut` to move it into the lambda.
  ///   auto moc = makeMoveOnCopy(mut);
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
  /// If you just want to construct a `MutableStore` with type deduction, simply
  /// use `makeMutableStore`.
  template<typename T>
  MutableStore<traits::Decay<T>, traits::Decay<T>*> makeMutableStoreFwd(T&& t)
  {
    return detail::makeMutableStoreFwdImpl(std::forward<T>(t), std::is_rvalue_reference<T&&>{});
  }

} // namespace qi

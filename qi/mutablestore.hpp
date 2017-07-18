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
    struct IsReferenceWrapper : traits::False
    {
    };

    template<typename T>
    struct IsReferenceWrapper<std::reference_wrapper<T>> : traits::True
    {
    };

    template<typename T>
    using EnableIfReferenceWrapper = traits::EnableIf<detail::IsReferenceWrapper<traits::Decay<T>>::value>;

    template<typename T>
    using EnableIfNotReferenceWrapper = traits::EnableIf<!detail::IsReferenceWrapper<traits::Decay<T>>::value>;

    template<typename T>
    using RefType = typename T::type;
  } // namespace detail

  /// Helper function to perform type deduction for constructing a MutableStore.
  ///
  /// If a `std::reference_wrapper` is passed, `t`'s address is stored inside the mutable.
  /// Otherwise, `t` is forwarded inside the mutable.
  ///
  /// Example: Storing an `int` by value
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int i = 5;
  /// auto m = makeMutableStore(i);
  /// ++(*m); // does not modify `i`
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Storing an `int` by reference
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// int i = 5;
  /// auto m = makeMutableStore(std::ref(i));
  /// ++(*m); // now: i == 6
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template<typename T>
  MutableStore<traits::Decay<T>, traits::Decay<T>*> makeMutableStore(T&& t,
    detail::EnableIfNotReferenceWrapper<T>* = {})
  {
    using D = traits::Decay<T>;
    return MutableStore<D, D*>{fwd<T>(t)};
  }

  template<typename T>
  MutableStore<detail::RefType<T>, detail::RefType<T>*> makeMutableStore(T&& t,
    detail::EnableIfReferenceWrapper<T>* = {})
  {
    using D = detail::RefType<T>;
    return MutableStore<D, D*>{&t.get()};
  }

} // namespace qi

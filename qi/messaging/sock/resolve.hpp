#pragma once
#ifndef _QI_SOCK_RESOLVE_HPP
#define _QI_SOCK_RESOLVE_HPP
#include <string>
#include <qi/messaging/sock/concept.hpp>
#include <qi/type/traits.hpp>
#include <qi/messaging/sock/traits.hpp>
#include <qi/messaging/sock/error.hpp>
#include <qi/messaging/sock/option.hpp>
#include <qi/url.hpp>
#include <qi/os.hpp>
#include <qi/functional.hpp>
#include <qi/macroregular.hpp>

/// @file
/// Contains functions and types related to URL resolve.

namespace qi { namespace sock {
  /// Resolve the given URL and give back a list of endpoints via a callback.
  ///
  /// The URL format must be: scheme://host:port
  /// Example: tcp://10.11.12.13:12345
  ///
  /// Lemma ResolveUrlList.0:
  ///   If this object is destroyed before the callback has been called, the
  ///   callback will be called with the error given by operationAborted<ErrorCode<N>>.
  ///
  /// Lemma ResolveUrlList.1:
  ///   The iterator given to the callback remains valid even if the object is
  ///   destroyed (meaning you can still access the iterator value).
  ///
  /// Usage:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Network N
  /// ResolveUrlList resolve{io};
  /// resolve(url, [](ErrorCode<N> e, Iterator<Resolver<N>> it) {
  ///   if (e) {
  ///     // handle error
  ///   } else {
  ///     Iterator<Resolver<N>> itEnd;
  ///     while (it != itEnd) {
  ///       // Use it->endpoint() for example.
  ///       ++it;
  ///     }
  ///   }
  /// });
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Regular if Resolver<N> is Regular.
  ///
  /// Network N
  template<typename N>
  class ResolveUrlList
  {
    Resolver<N> _resolver;
  public:
  // QuasiRegular (if Resolver<N> is QuasiRegular):
    QI_GENERATE_FRIEND_REGULAR_OPS_1(ResolveUrlList, _resolver)
  // Custom:
    explicit ResolveUrlList(IoService<N>& io)
      : _resolver{io}
    {
    }
    IoService<N>& getIoService()
    {
      return _resolver.get_io_service();
    }
  // Procedure:
    /// Network N,
    /// Procedure<void (ErrorCode<N>, Iterator<Resolver<N>>)> Proc,
    /// Procedure<void (Resolver<N>&)> Proc1
    template<typename Proc, typename Proc1 = PolymorphicConstantFunction<void>>
    void operator()(const Url& url, Proc onComplete, Proc1 setupStop = Proc1{})
    {
      if (!url.isValid() || url.port() == 0)
      {
        onComplete(badAddress<ErrorCode<N>>(), {});
        return;
      }
      qiLogVerbose(logCategory()) << "(ResolverUrlList)" << this << ": Trying to connect to " << url.host() << ":" << url.port();
      Query<Resolver<N>> query(url.host(), os::to_string(url.port())
#if !BOOST_OS_ANDROID
        , Query<Resolver<N>>::all_matching
#endif
      );
      _resolver.async_resolve(query, onComplete);
      setupStop(_resolver);
    }
  };

  namespace detail
  {
    /// Precondition: readableBoundedRange(b, e)
    ///
    /// Iterator<Entry<Resolver<N>>> I
    template<typename I>
    auto findFirstValidIfAny(I b, const I& e, IpV6Enabled ipV6)
        -> boost::optional<traits::Decay<decltype(*b)>>
    {
      using Entry = traits::Decay<decltype(*b)>;
      if (!(*ipV6))
      {
        b = std::find_if(b, e, [](const Entry& entry) {
          return !entry.endpoint().address().is_v6();
        });
      }
      using O = boost::optional<Entry>;
      return b == e ? O{} : O{*b};
    }
  } // namespace detail

  /// Resolve the given url and give back the first endpoint, if any, skipping
  /// ipV6 endpoint if asked for.
  ///
  /// See `ResolveUrlList` for the url format.
  ///
  /// Lemma ResolveUrl.0:
  ///   If this object is destroyed before the callback has been called, the
  ///   callback will be called with the error given by operationAborted<ErrorCode<N>>.
  ///
  /// Lemma ResolveUrl.1:
  ///   The iterator given to the callback remains valid even if the object is
  ///   destroyed (meaning you can still access the iterator value).
  ///
  /// Usage:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Network N
  /// ResolveUrl resolve{io};
  /// resolve(url, IpV6Enabled{false},
  ///   [](ErrorCode<N> err, boost::optional<Entry<Resolver<N>>> optionalEntry) {
  ///     if (err) {
  ///       // handle error
  ///     } else {
  ///       if (optionalEntry) {
  ///         auto entry = optionalEntry.value();
  ///         // ...
  ///       }
  ///     }
  /// });
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Regular if ResolveUrlList<N> is Regular.
  ///
  /// Network N
  template<typename N>
  class ResolveUrl
  {
    using OptionalEntry = boost::optional<Entry<Resolver<N>>>;
    ResolveUrlList<N> _resolve;
  public:
  // QuasiRegular (if ResolveUrlList<N> is QuasiRegular):
    QI_GENERATE_FRIEND_REGULAR_OPS_1(ResolveUrl, _resolve)
  // Custom:
    explicit ResolveUrl(IoService<N>& io)
      : _resolve{io}
    {
    }
    IoService<N>& getIoService()
    {
      return _resolve.getIoService();
    }
  // Procedure:
    /// Procedure<void (ErrorCode<N>, OptionalEntry)> Proc,
    /// Procedure<void (Resolver<N>&)> Proc1
    template<typename Proc, typename Proc1 = PolymorphicConstantFunction<void>>
    void operator()(const Url& url, IpV6Enabled ipV6, Proc onComplete, Proc1 setupStop = Proc1{})
    {
      _resolve(url,
        [=](const ErrorCode<N>& erc, Iterator<Resolver<N>> it) mutable {
          if (erc)
          {
            onComplete(erc, OptionalEntry{});
            return;
          }
          decltype(it) itEnd;
          onComplete(erc, detail::findFirstValidIfAny(it, itEnd, ipV6));
        },
        setupStop);
    }
  };
}} // namespace qi::sock

#endif // _QI_SOCK_RESOLVE_HPP

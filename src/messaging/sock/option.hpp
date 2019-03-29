#pragma once
#ifndef _QI_SOCK_OPTION_HPP
#define _QI_SOCK_OPTION_HPP
#include <limits>
#include <boost/optional.hpp>
#include <ka/typetraits.hpp>
#include <ka/macroregular.hpp>
#include <qi/log.hpp>
#include "concept.hpp"
#include "traits.hpp"
#include "socketptr.hpp"

/// @file
/// Contains option types (ssl, ipV6) and functions to set options on a socket.

namespace qi { namespace sock {

  inline char const* logCategory()
  {
    return "qimessaging.messagesocket";
  }

  /// Option that, if true, means that ssl is enabled.
  class SslEnabled
  {
    bool value;
  public:
  // Regular:
    SslEnabled(bool v = true) : value(v) {}
    KA_GENERATE_FRIEND_REGULAR_OPS_1(SslEnabled, value)
  // Readable:
    bool operator*() const
    {
      return value;
    }
  };

  /// Option that, if true, means that ipV6 is enabled.
  class IpV6Enabled
  {
    bool value;
  public:
  // Regular:
    IpV6Enabled(bool v = true) : value(v) {}
    KA_GENERATE_FRIEND_REGULAR_OPS_1(IpV6Enabled, value)
  // Readable:
    bool operator*() const
    {
      return value;
    }
  };

  /// Option that, if true, means that an address can be reused.
  /// Can be used in the context of a socket acceptor.
  class ReuseAddressEnabled
  {
    bool value;
  public:
  // Regular:
    ReuseAddressEnabled(bool v = true) : value(v) {}
    KA_GENERATE_FRIEND_REGULAR_OPS_1(ReuseAddressEnabled, value)
  // Readable:
    bool operator*() const
    {
      return value;
    }
  };

  /// Set default options on a socket, including the timeout.
  ///
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N,
  ///   Mutable<S> S
  template<typename N, typename S>
  void setSocketOptions(S socket, const boost::optional<Seconds>& timeout)
  {
    // Transmit each Message without delay
    try
    {
      (*socket).lowest_layer().set_option(sock::SocketOptionNoDelay<N>{true});
    }
    catch (const std::exception& e)
    {
      qiLogWarning(logCategory()) << "Can't set no_delay option: " << e.what();
    }

    // Feature disabled.
    if (!timeout) return;

    // Enable TCP keepalive for faster timeout detection.
    // We cannot properly honor a timeout less than 10 seconds.
    using I = ka::Decay<decltype(timeout.value().count())>;
    auto ajustedTimeout = std::max(timeout.value().count(), I(10));
    auto handle = (*socket).lowest_layer().native_handle();
    static const auto intMax = std::numeric_limits<int>::max();
    if (ajustedTimeout > I(intMax))
    {
      qiLogWarning(logCategory()) << "setSocketOptions: timeout too big for an int. "
        "Truncated to int max value (" << intMax << ")";
      ajustedTimeout = intMax;
    }
    N::setSocketNativeOptions(handle, static_cast<int>(ajustedTimeout));
  }
}} // namespace qi::sock

#endif // _QI_SOCK_OPTION_HPP

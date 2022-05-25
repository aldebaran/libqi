/*
**  Copyright (C) 2022 Aldebaran Robotics
**  See COPYING for the license
*/

#pragma once

#ifndef QI_MESSAGING_TCPSCHEME_HPP
#define QI_MESSAGING_TCPSCHEME_HPP

#include <boost/optional.hpp>
#include <qi/url.hpp>
#include <qi/uri.hpp>
#include <string>

namespace qi
{

/// Set of TCP-based URI schemes supported by the library.
enum class QI_API TcpScheme
{
  /// The raw TCP URI scheme is used when no TLS-based encryption must be
  /// done by sockets.
  /// This corresponds to the "tcp" URI scheme.
  Raw,
  /// The TCP with TLS URI scheme is used when TLS-based encryption must be
  /// done by sockets. Certificate validation is disabled for the server.
  /// At the moment, it is also disabled for the client, but this is subject
  /// to change in the future.
  /// This corresponds to the "tcps" URI scheme.
  Tls,
  /// The TCP with mutual TLS URI scheme is used when TLS-based encryption
  /// must be done by sockets with certificate validation done by both peers.
  /// This corresponds to the "tcpsm" URI scheme.
  MutualTls,
};

/// Returns the TCP scheme corresponding to the scheme as a string, or an empty optional if the
/// scheme is not one of the TCP schemes.
QI_API boost::optional<TcpScheme> tcpSchemeFromUriScheme(const std::string& scheme);

/// Returns the TCP scheme corresponding to the "protocol" (i.e. a scheme) of the URL or an empty
/// optional if its "protocol" is missing or is not one of the TCP schemes.
QI_API boost::optional<TcpScheme> tcpScheme(const Url& url);

/// Returns the TCP scheme as a string. See the `TcpScheme` type for the values equivalence.
///
/// Invariant (tcpSchemeFromUriScheme and to_string are "quasi-inverses"):
///   With TcpScheme tcpScheme, std::string strScheme:
///     *tcpSchemeFromUriScheme(to_string(tcpScheme)) == tcpScheme
///  && (!tcpSchemeFromUriScheme(strScheme).has_value()
///     || to_string(*tcpSchemeFromUriScheme(strScheme)) == strScheme)
///
/// @pre The scheme is one of the values of the enumeration. This is verified by assertion, but will
/// throw a `std::domain_error` if assertions are disabled.
QI_API std::string to_string(TcpScheme scheme);

/// Returns true if the TCP scheme forces TLS-based encryption when used.
QI_API bool isWithTls(TcpScheme scheme);

} // namespace qi

#endif // QI_MESSAGING_TCPSCHEME_HPP

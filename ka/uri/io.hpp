#ifndef KA_URI_IO_HPP
#define KA_URI_IO_HPP
#pragma once

#include "uri_fwd.hpp"
#include <ostream>
#include <sstream>

/// @file IO operations on URI types.
///
/// In this file:
///   - `Outputs "expr1expr2"` means that the function outputs the concatenation
///     of "expr1" and "expr2".
///   - `Outputs "[expr]"` means that the function outputs something if all the
///     element in the expression are defined.
///   - An expression can be a member of the parameter of the function or a
///     string. No disambiguation between the two is specified as most cases
///     should be trivially understandable.

namespace ka {
  namespace detail {
    struct insert {
      std::ostream& os;
      char const* prefix;
      char const* suffix;

      explicit insert(std::ostream& os, char const* prefix = "", char const* suffix = "")
        : os(os), prefix(prefix), suffix(suffix) {
      }

      template<typename T>
      auto operator()(T const& t) -> void {
        os << prefix << t << suffix;
      }
    };
  } // namespace detail

  /// Outputs "username[:password]" into the stream.
  inline std::ostream& operator<<(std::ostream& os, uri_userinfo_t const& userinfo) {
    os << userinfo.username();
    userinfo.password().fmap(detail::insert{os, ":"});
    return os;
  }

  /// See `operator<<`.
  inline auto to_string(uri_userinfo_t const& userinfo) -> std::string {
    std::ostringstream oss;
    oss << userinfo;
    return oss.str();
  }

  /// Outputs "[userinfo@]host[:port]" into the stream.
  inline std::ostream& operator<<(std::ostream& os, uri_authority_t const& auth) {
    using detail::insert;
    auth.userinfo().fmap(insert{os, "", "@"});
    os << auth.host();
    auth.port().fmap(insert{os, ":"});
    return os;
  }

  /// See `operator<<`.
  inline auto to_string(uri_authority_t const& auth) -> std::string {
    std::ostringstream oss;
    oss << auth;
    return oss.str();
  }

  /// Outputs "scheme:[//authority]path[?query][#fragment]"
  inline std::ostream& operator<<(std::ostream& os, uri_t const& uri) {
    using detail::insert;
    os << uri.scheme() << ':';
    uri.authority().fmap(insert{os, "//"});
    os << uri.path();
    uri.query().fmap(insert{os, "?"});
    uri.fragment().fmap(insert{os, "#"});
    return os;
  }

  /// See `operator<<`.
  inline auto to_string(uri_t const& uri) -> std::string {
    std::ostringstream oss;
    oss << uri;
    return oss.str();
  }
} // namespace ka

#endif

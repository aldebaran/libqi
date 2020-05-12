#ifndef KA_URI_URI_FWD_HPP
#define KA_URI_URI_FWD_HPP
#pragma once

#include "../macroregular.hpp"
#include "../opt.hpp"
#include <string>

//@file For the `meaning` clauses, see `spec:/sbre/framework/2020/b`.

namespace ka {
  struct uri_userinfo_t;

  namespace detail_uri {
    template<typename... Args>
    auto unchecked_uri_userinfo(Args&&... args) -> uri_userinfo_t;
  }

  /// An URI authority user info according to RFC 3986.
  struct uri_userinfo_t {
  // Regular:
    uri_userinfo_t() = default;
    KA_GENERATE_FRIEND_REGULAR_OPS_2(uri_userinfo_t, username_, password_)

  // uri_authority_t:
    auto username() const -> std::string {
      return username_;
    }
    auto password() const -> opt_t<std::string> {
      return password_;
    }
  private:
    explicit uri_userinfo_t(
      std::string username, opt_t<std::string> password = {})
        : username_(mv(username))
        , password_(mv(password)) {
    }

    template<typename... Args>
    friend auto ka::detail_uri::unchecked_uri_userinfo(Args&&... args) -> uri_userinfo_t;

    std::string username_;
    opt_t<std::string> password_;
  };

  template<typename... Args>
  auto detail_uri::unchecked_uri_userinfo(Args&&... args) -> uri_userinfo_t {
    return uri_userinfo_t{ fwd<Args>(args)... };
  }

  inline auto username(uri_userinfo_t const& ui) -> std::string {
    return ui.username();
  }

  inline auto password(uri_userinfo_t const& ui) -> opt_t<std::string> {
    return ui.password();
  }

  struct uri_authority_t;

  namespace detail_uri {
    template<typename... Args>
    auto unchecked_uri_authority(Args&&... args) -> uri_authority_t;
  }

  /// An URI authority according to RFC 3986.
  struct uri_authority_t {
  // Regular:
    uri_authority_t() = default;

    KA_GENERATE_FRIEND_REGULAR_OPS_3(uri_authority_t, userinfo_, host_, port_)

    auto userinfo() const -> opt_t<uri_userinfo_t> {
      return userinfo_;
    }

    auto host() const -> std::string {
      return host_;
    }

    auto port() const -> opt_t<std::uint16_t> {
      return port_;
    }

  private:
    explicit uri_authority_t(
      opt_t<uri_userinfo_t> userinfo,
      std::string host = {},
      opt_t<std::uint16_t> port = {})
      : userinfo_(mv(userinfo))
      , host_(mv(host))
      , port_(mv(port)) {
    }

    template<typename... Args>
    friend auto ka::detail_uri::unchecked_uri_authority(Args&&... args) -> uri_authority_t;

    opt_t<uri_userinfo_t> userinfo_;
    std::string host_;
    opt_t<std::uint16_t> port_;
  };

  template<typename... Args>
  auto detail_uri::unchecked_uri_authority(Args&&... args) -> uri_authority_t {
    return uri_authority_t{ fwd<Args>(args)... };
  }

  inline auto userinfo(uri_authority_t const& auth) -> opt_t<uri_userinfo_t> {
    return auth.userinfo();
  }

  inline auto host(uri_authority_t const& auth) -> std::string {
    return auth.host();
  }

  inline auto port(uri_authority_t const& auth) -> opt_t<std::uint16_t> {
    return auth.port();
  }

  struct uri_t;

  namespace detail_uri {
    template<typename... Args>
    auto unchecked_uri(Args&&... args) -> uri_t;
  }

  /// meaning(uri_t) = UC, with C = meaning(char)
  ///
  /// An URI according to RFC 3986.
  struct uri_t {
  // Regular:
    /// Constructs an incomplete URI.
    uri_t() = default;

    KA_GENERATE_FRIEND_REGULAR_OPS_5(uri_t, scheme_, authority_, path_, query_, fragment_)

    auto scheme() const -> std::string {
      return scheme_;
    }

    auto authority() const -> opt_t<uri_authority_t> {
      return authority_;
    }

    auto path() const -> std::string {
      return path_;
    }

    auto query() const -> opt_t<std::string> {
      return query_;
    }

    auto fragment() const -> opt_t<std::string> {
      return fragment_;
    }

  private:
    explicit uri_t(std::string scheme,
      opt_t<uri_authority_t> authority = {},
      std::string path = {},
      opt_t<std::string> query = {},
      opt_t<std::string> fragment = {})
      : scheme_(mv(scheme))
      , authority_(mv(authority))
      , path_(mv(path))
      , query_(mv(query))
      , fragment_(mv(fragment)) {
    }

    template<typename... Args>
    friend auto ka::detail_uri::unchecked_uri(Args&&... args) -> uri_t;

    std::string scheme_;
    opt_t<uri_authority_t> authority_;
    std::string path_;
    opt_t<std::string> query_;
    opt_t<std::string> fragment_;
  };

  template<typename... Args>
  auto detail_uri::unchecked_uri(Args&&... args) -> uri_t {
    return uri_t{ fwd<Args>(args)... };
  }

  inline auto scheme(uri_t const& uri) -> std::string {
    return uri.scheme();
  }

  inline auto authority(uri_t const& uri) -> opt_t<uri_authority_t> {
    return uri.authority();
  }

  inline auto path(uri_t const& uri) -> std::string {
    return uri.path();
  }

  inline auto query(uri_t const& uri) -> opt_t<std::string> {
    return uri.query();
  }

  inline auto fragment(uri_t const& uri) -> opt_t<std::string> {
    return uri.fragment();
  }
} // namespace ka

#endif // KA_URI_URI_FWD_HPP

#ifndef KA_TEST_URI_COMMON_HPP
#define KA_TEST_URI_COMMON_HPP
#pragma once

#include <iostream>
#include <ka/uri/uri_fwd.hpp>

namespace ka {
  // Used by gtest.
  inline void PrintTo(const uri_userinfo_t& ui, std::ostream* os) {
      *os << "userinfo: { username: \"" << ui.username() << "\"";
      ui.password().fmap([&](std::string const& pwd) {
        *os << ", password: \"" << pwd << "\"";
      });
      *os << " }";
  }

  // Used by gtest.
  inline void PrintTo(const uri_authority_t& auth, std::ostream* os) {
    *os << "authority: { host: \"" << auth.host() << "\"";
    auth.userinfo().fmap([&](uri_userinfo_t const& ui) {
      *os << ", ";
      PrintTo(ui, os);
    });
    auth.port().fmap([&](std::uint16_t port) {
      *os << ", port: " << port;
    });
    *os << " }";
  }

  // Used by gtest.
  inline void PrintTo(const uri_t& uri, std::ostream* os) {
    *os << "uri: { scheme: \"" << uri.scheme()
        << "\", path: \"" << uri.path() << "\"";
    uri.authority().fmap([&](uri_authority_t const& auth) {
      *os << ", ";
      PrintTo(auth, os);
    });
    uri.query().fmap([&](std::string const& query) {
      *os << ", query: \"" << query << "\"";
    });
    uri.fragment().fmap([&](std::string const& fragment) {
      *os << ", fragment: \"" << fragment << "\"";
    });
    *os << " }";
  }

  namespace test {
    using detail_uri::unchecked_uri;
    using detail_uri::unchecked_uri_authority;
    using detail_uri::unchecked_uri_userinfo;

    // The two following arrays must be kept in sync.
    static uri_t const uri_list[] = {
      unchecked_uri("ftp",
                    opt(unchecked_uri_authority(opt_t<uri_userinfo_t>{}, "ftp.is.co.za")),
                    "/rfc/rfc1808.txt"),
      unchecked_uri("http",
                    opt(unchecked_uri_authority(opt_t<uri_userinfo_t>{}, "www.ietf.org")),
                    "/rfc/rfc2396.txt"),
      unchecked_uri("ldap",
                    opt(unchecked_uri_authority(opt_t<uri_userinfo_t>{}, "[2001:db8::7]")),
                    "/c=GB",
                    opt(std::string("objectClass?one"))),
      unchecked_uri("news", opt_t<uri_authority_t>{}, "comp.infosystems.www.servers.unix"),
      unchecked_uri("tel", opt_t<uri_authority_t>{}, "+1-816-555-1212"),
      unchecked_uri(
        "telnet",
        opt(unchecked_uri_authority(opt_t<uri_userinfo_t>{}, "192.0.2.16", opt<std::uint16_t>(80))),
        "/"),
      unchecked_uri("urn",
                    opt_t<uri_authority_t>{},
                    "oasis:names:specification:docbook:dtd:xml:4.1.2"),
      unchecked_uri("http",
                    opt(unchecked_uri_authority(opt_t<uri_userinfo_t>{}, "www.google.com"))),
      unchecked_uri(
        "http",
        opt(unchecked_uri_authority(opt(unchecked_uri_userinfo("foo", opt(std::string("bar")))),
                                    "w1.superman.com")),
        "/very/long/path.html",
        opt(std::string("p1=v1&p2=v2")),
        opt(std::string("more-details"))),
      unchecked_uri("https",
                    opt(unchecked_uri_authority(opt_t<uri_userinfo_t>{},
                                                "secured.com",
                                                opt<std::uint16_t>(443)))),
      unchecked_uri("ftp",
                    opt(unchecked_uri_authority(opt_t<uri_userinfo_t>{}, "ftp.bogus.com")),
                    "/~some/path/to/a/file.txt"),
      unchecked_uri("s",
                    opt_t<uri_authority_t>{},
                    ":::::::::::::::::::::::::::::::::::::::::::::::::::::"),
      unchecked_uri("s", opt_t<uri_authority_t>{}, std::string{}, opt(std::string{})),
    };
    static std::string const uri_strings[] = {
      "ftp://ftp.is.co.za/rfc/rfc1808.txt",
      "http://www.ietf.org/rfc/rfc2396.txt",
      "ldap://[2001:db8::7]/c=GB?objectClass?one",
      "news:comp.infosystems.www.servers.unix",
      "tel:+1-816-555-1212",
      "telnet://192.0.2.16:80/",
      "urn:oasis:names:specification:docbook:dtd:xml:4.1.2",
      "http://www.google.com",
      "http://foo:bar@w1.superman.com/very/long/path.html?p1=v1&p2=v2#more-details",
      "https://secured.com:443",
      "ftp://ftp.bogus.com/~some/path/to/a/file.txt",
      "s::::::::::::::::::::::::::::::::::::::::::::::::::::::",
      "s:?",
    };

    static std::string const base_bad_uri_list[] = {
      // No scheme
      "",
      "   ",
      "s",
      "5",
    };

    static std::string const bad_schemes[] = {
      // empty scheme
      "",
      // invalid characters in scheme
      "+sc",
      "6sc",
      "s?c",
      "séc",
      "ásc",
    };

    inline std::vector<std::string> bad_uri_list() {
      std::vector<std::string> res(std::begin(base_bad_uri_list), std::end(base_bad_uri_list));
      for (auto const& scheme : bad_schemes) {
        for (auto const& query : { "", "?b" }) {
          for (auto const& fragment : { "", "#c" }) {
            res.push_back(scheme + ":" + query + fragment); // empty path
            res.push_back(scheme + ":p" + query + fragment); // non empty path
            res.push_back(scheme + ":/a/b/c" + query + fragment); // absolute path
          }
        }
      }
      return res;
    }
  } // namespace test
} // namespace ka

#endif // KA_TEST_URI_COMMON_HPP

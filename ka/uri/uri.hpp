#ifndef KA_URI_URI_HPP
#define KA_URI_URI_HPP
#pragma once

#include "uri_fwd.hpp"
#include "parsing.hpp"
#include <stdexcept>

namespace ka {
  /// @file URI construction functions.

  /// Parses a URI authority userinfo from a range of characters.
  ///
  /// ForwardIterator<char> I
  template<typename I>
  auto uri_userinfo(I b, I e) noexcept -> parse::res_t<uri_userinfo_t, I> {
    auto res = detail_uri::parsing::grammar::userinfo(b, e);
    if (res.empty()) {
      return parse::err(type_t<uri_userinfo_t>{}, b);
    }
    auto it = iter(res);
    return parse::ok(*mv(res), mv(it));
  }

  /// Parses a URI authority from a range of characters.
  ///
  /// ForwardIterator<char> I
  template<typename I>
  auto uri_authority(I b, I e) noexcept -> parse::res_t<uri_authority_t, I> {
    auto res = detail_uri::parsing::grammar::authority(b, e);
    if (res.empty()) {
      return parse::err(type_t<uri_authority_t>{}, b);
    }
    auto it = iter(res);
    return parse::ok(*mv(res), mv(it));
  }

  /// Parses a URI from a range of characters.
  ///
  /// The URI components are normalized according to RFC 3986 "Syntax-Based Normalization"
  /// (section 6.2.2).
  ///
  /// ForwardIterator<char> I
  template<typename I>
  auto uri(I b, I e) noexcept -> parse::res_t<uri_t, I> {
    auto res = detail_uri::parsing::grammar::uri(b, e);
    if (res.empty()) {
      return parse::err(type_t<uri_t>{}, b);
    }
    auto it = iter(res);
    return parse::ok(*mv(res), mv(it));
  }
} // namespace ka

#endif // KA_URI_URI_HPP

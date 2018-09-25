#pragma once
#ifndef _QI_SOCK_SSLCONTEXTPTR_HPP
#define _QI_SOCK_SSLCONTEXTPTR_HPP

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <ka/utility.hpp>
#include "traits.hpp"

/// @file
/// Contains SSL context pointer traits and helper functions used in the sock namespace.

namespace qi { namespace sock {

  template<typename N>
  using SslContextPtr = boost::shared_ptr<SslContext<N>>;

  /// Helper function to deduce types for `SslContextPtr`.
  ///
  /// Network N
  template<typename N, typename... Args>
  SslContextPtr<N> makeSslContextPtr(Args&&... args)
  {
    return boost::make_shared<SslContext<N>>(ka::fwd<Args>(args)...);
  }
}} // namespace qi::sock

#endif // _QI_SOCK_SSLCONTEXTPTR_HPP

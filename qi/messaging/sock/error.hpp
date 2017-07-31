#pragma once
#ifndef _QI_SOCK_ERROR_HPP
#define _QI_SOCK_ERROR_HPP
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp>

/// @file
/// Contains functions that implement the NetErrorCode concept for boost::system::error_code.
///
/// TODO: Replace these functions by template variables when C++14 is available.
///
/// See concept.hpp

namespace qi { namespace sock {
  template<typename Error>
  Error success();

  template<>
  inline boost::system::error_code success<boost::system::error_code>()
  {
    return {};
  }

  template<typename Error>
  Error badAddress();

  template<>
  inline boost::system::error_code badAddress<boost::system::error_code>()
  {
    using namespace boost::system;
    return error_code{errc::bad_address, system_category()};
  }

  template<typename Error>
  Error networkUnreachable();

  template<>
  inline boost::system::error_code networkUnreachable<boost::system::error_code>()
  {
    using namespace boost::system;
    return error_code{errc::network_unreachable, system_category()};
  }

  template<typename Error>
  Error ownerDead();

  template<>
  inline boost::system::error_code ownerDead<boost::system::error_code>()
  {
    using namespace boost::system;
    return error_code{errc::owner_dead, system_category()};
  }

  template<typename Error>
  Error operationAborted();

  template<>
  inline boost::system::error_code operationAborted<boost::system::error_code>()
  {
    using namespace boost;
    return system::error_code{asio::error::operation_aborted, system::system_category()};
  }

  template<typename Error>
  Error hostNotFound();

  template<>
  inline boost::system::error_code hostNotFound<boost::system::error_code>()
  {
    using namespace boost;
    return system::error_code{asio::error::host_not_found, system::system_category()};
  }

  template<typename Error>
  Error fault();

  template<>
  inline boost::system::error_code fault<boost::system::error_code>()
  {
    using namespace boost;
    return system::error_code{asio::error::fault, system::system_category()};
  }

  template<typename Error>
  Error messageSize();

  template<>
  inline boost::system::error_code messageSize<boost::system::error_code>()
  {
    using namespace boost;
    return system::error_code{asio::error::message_size, system::system_category()};
  }

  template<typename Error>
  Error connectionRefused();

  template<>
  inline boost::system::error_code connectionRefused<boost::system::error_code>()
  {
    using namespace boost;
    return system::error_code{asio::error::connection_refused, system::system_category()};
  }

  template<typename Error>
  Error shutdown();

  template<>
  inline boost::system::error_code shutdown<boost::system::error_code>()
  {
    using namespace boost;
    return system::error_code{asio::error::shut_down, system::system_category()};
  }

}} // namespace qi::sock

#endif // _QI_SOCK_ERROR_HPP

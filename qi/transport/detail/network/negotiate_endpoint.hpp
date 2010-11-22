#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_NEGOTIATE_ENDPOINT_HPP__
#define   __QI_MESSAGING_DETAIL_NEGOTIATE_ENDPOINT_HPP__
#include <qi/transport/detail/network/endpoint_context.hpp>

namespace qi {
  namespace detail {
    /// <summary> Returns a fully qualified endpoint for connecting a client and a server </summary>
    std::string negotiateEndpoint(const EndpointContext& clientContext, const EndpointContext serverContext);
  }
}
#endif // __QI_MESSAGING_DETAIL_NEGOTIATE_ENDPOINT_HPP__


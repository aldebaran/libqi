/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_MESSAGING_MESSAGING_CLIENT_HPP
#define QI_MESSAGING_MESSAGING_CLIENT_HPP

#include <qi/messaging/generic_client.hpp>
#include <qi/messaging/call_definition_serialization.hxx>
#include <qi/messaging/result_definition_serialization.hxx>

namespace qi {
  namespace messaging {
    typedef GenericClient<qi::messaging::CallDefinition, qi::messaging::ResultDefinition> Client;
  }
}

#endif  // QI_MESSAGING_MESSAGING_CLIENT_HPP

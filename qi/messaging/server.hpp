/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_MESSAGING_SERVER_HPP
#define QI_MESSAGING_SERVER_HPP

#include <qi/messaging/generic_server.hpp>
#include <qi/messaging/call_definition_serialization.hxx>
#include <qi/messaging/result_definition_serialization.hxx>

namespace qi {
  namespace Messaging {
    typedef GenericServer<qi::Messaging::CallDefinition, qi::Messaging::ResultDefinition> Server;
  }
}

#endif // QI_MESSAGING_SERVER_HPP

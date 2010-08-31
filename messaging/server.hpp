/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_SERVER_HPP
#define AL_MESSAGING_SERVER_HPP

#include <alcommon-ng/messaging/generic_server.hpp>
#include <alcommon-ng/messaging/call_definition_serialization.hxx>
#include <alcommon-ng/messaging/result_definition_serialization.hxx>

namespace AL {
  namespace Messaging {
    typedef GenericServer<AL::Messaging::CallDefinition, AL::Messaging::ResultDefinition> Server;
  }
}

#endif // AL_MESSAGING_SERVER_HPP

/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_MESSAGING_CLIENT_HPP
#define AL_MESSAGING_MESSAGING_CLIENT_HPP

#include <alcommon-ng/messaging/generic_client.hpp>
#include <alcommon-ng/messaging/call_definition_serialization.hxx>
#include <alcommon-ng/messaging/result_definition_serialization.hxx>

namespace AL {
  namespace Messaging {
    typedef GenericClient<AL::Messaging::CallDefinition, AL::Messaging::ResultDefinition> Client;
  }
}

#endif  // AL_MESSAGING_MESSAGING_CLIENT_HPP

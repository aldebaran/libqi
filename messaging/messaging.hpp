/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_MESSAGING_HPP
#define AL_MESSAGING_MESSAGING_HPP

# include <alcommon-ng/messaging/client.hpp>
# include <alcommon-ng/messaging/server.hpp>
# include <alcommon-ng/messaging/messagehandler.hpp>

namespace AL {
  namespace Messaging {
    typedef Client<AL::Messaging::CallDefinition, AL::Messaging::ResultDefinition> DefaultClient;
    typedef Server<AL::Messaging::CallDefinition, AL::Messaging::ResultDefinition> DefaultServer;
    typedef MessageHandler<AL::Messaging::CallDefinition, AL::Messaging::ResultDefinition> DefaultMessageHandler;
  }
}

#endif // AL_MESSAGING_MESSAGING_HPP

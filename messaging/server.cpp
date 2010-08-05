/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/messaging/server.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>
#include <alcommon-ng/serialization/serialization.h>
#include <alcommon-ng/transport/transport.hpp>

using namespace AL::Serialization;

namespace AL {
  namespace Messaging {

    Server::Server(const std::string &address)
    {
      _server = new AL::Transport::ZMQServer(address);
      _server->setDataHandler(this);
    }

    void Server::onData(const std::string &data, std::string &result)
    {
      CallDefinition def = Serializer::deserialize<CallDefinition>(data);
      boost::shared_ptr<ResultDefinition> res;

      res = _onMessageDelegate->onMessage(def);
      assert(res);
      result = Serializer::serialize(*res);
    }

    void Server::run()
    {
      _server->run();
    }

  }
}

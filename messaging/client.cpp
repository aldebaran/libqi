/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <sstream>
#include <alcommon-ng/serialization/iarchive.hpp>
#include <alcommon-ng/serialization/oarchive.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <alcommon-ng/messaging/client.hpp>
#include <alcommon-ng/transport/zeromq/zmqclient.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>
#include <alcommon-ng/serialization/serialization.h>

using namespace AL::Serialization;

namespace AL {
  namespace Messaging {

    Client::Client(const std::string &address)
    {
      _client = new AL::Transport::ZMQClient(address);
    }

    boost::shared_ptr<ResultDefinition> Client::send(CallDefinition &def)
    {
      std::string tosend = Serializer::serialize(def);
      std::string torecv;

      _client->send(tosend, torecv);
      return Serializer::deserializeToPtr<ResultDefinition>(torecv);
    }

  }
}

/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/


#include <sstream>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <alcommon-ng/serialization/oarchive.hpp>
#include <alcommon-ng/serialization/iarchive.hpp>
#include <alcommon-ng/messaging/server.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>

#include <alcommon-ng/transport/transport.hpp>

namespace AL {
  namespace Messaging {

    void marshallResult(const ResultDefinition &result, std::string &msg)
    {
      std::stringstream  outstream;
      OArchive           oarchive(outstream);

      oarchive << result;
      msg = outstream.str();
    }

    AL::ALPtr<CallDefinition> unmarshallCall(const std::string &msg, CallDefinition &def)
    {
      boost::interprocess::bufferstream bstream((char *)msg.data(), msg.size());
      IArchive                          archive(bstream);

      archive >> def;
    }

    Server::Server(const std::string &address)
    {
      _server = new AL::Transport::ZMQServer(address);
      _server->setOnDataDelegate(this);
    }

    void Server::onData(const std::string &data, std::string &result)
    {
      CallDefinition              def;
      AL::ALPtr<ResultDefinition> res;

      unmarshallCall(data, def);
      res = _onMessageDelegate->onMessage(def);
      assert(res);
      marshallResult(*res, result);
    }

    void Server::run()
    {
      _server->run();
    }

  }
}

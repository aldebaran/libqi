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

namespace AL {
  namespace Messaging {

    void marshallCall(const CallDefinition &def, std::string &msg)
    {

      std::stringstream  outstream;
      OArchive           oarchive(outstream);

      oarchive << def;

      //copy the message content
      msg = outstream.str();
      std::cout << "marshallCall(" << msg.size() << ")" << std::endl;
    }

    void unmarshallResult(ResultDefinition &res, const std::string &result)
    {
      std::cout << "unmarshallResult(" << result.size() << ")" << std::endl;

      boost::interprocess::bufferstream bstream((char *)result.data(), result.size());
      IArchive                          archive(bstream);
      archive >> res;
    }


    Client::Client(const std::string &address)
    {
      _client = new AL::Transport::ZMQClient(address);
    }


    AL::ALPtr<ResultDefinition> Client::send(CallDefinition &def)
    {
      AL::ALPtr<ResultDefinition> res = AL::ALPtr<ResultDefinition>(new ResultDefinition());
      std::string                 tosend;
      std::string                 torecv;

      marshallCall(def, tosend);
      _client->send(tosend, torecv);
      unmarshallResult(*res, torecv);
      return res;
    }

  }
}

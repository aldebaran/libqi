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

namespace AL {
  namespace Messaging {
    void marshallResult(AL::ALPtr<ResultDefinition> result, std::string &msg)
    {
      std::stringstream  outstream;
      OArchive           oarchive(outstream);

      oarchive << (*result);
      msg = outstream.str();
    }

    void marshallCall(AL::ALPtr<CallDefinition> def, std::string &msg)
    {
      std::stringstream  outstream;
      OArchive           oarchive(outstream);

      oarchive << (*def);

      //copy the message content
      msg = outstream.str();
    }

    AL::ALPtr<CallDefinition> unmarshallCall(std::string &msg)
    {
      AL::ALPtr<CallDefinition>         def(new CallDefinition());
      boost::interprocess::bufferstream bstream((char *)msg.data(), msg.size());
      IArchive                          archive(bstream);

      archive >> *def;
      return def;
    }


  }
}

/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	AL_MESSAGING_GENERIC_CLIENT_HPP_
# define   	AL_MESSAGING_GENERIC_CLIENT_HPP_

#include <string>
#include <alcommon-ng/transport/client.hpp>
#include <alcommon-ng/transport/zeromq/zmqclient.hpp>
#include <alcommon-ng/serialization/serializer.hpp>
#include <allog/allog.h>

// temporary
//#include <alcommon-ng/messaging/call_definition_serialization.hxx>
//#include <alcommon-ng/messaging/result_definition_serialization.hxx>

namespace AL {
  namespace Messaging {

    template<typename T, typename R>
    class GenericClient {
    public:
      GenericClient() : initOK(false) {}

      bool connect(const std::string &address) {
        try {
          _client = new AL::Transport::ZMQClient(address);
          initOK = true;
        } catch(const std::exception& e) {
          alsdebug << "GenericClient failed to create client for address \"" << address << "\" Reason: " << e.what();
        }
         return initOK;
      }

      R send(const T &def)
      {
        if (! initOK) {
          alserror << "Attempt to use an unitialized client.";
          // TODO should throw
          R r;
          return r;
        }
        std::string tosend = AL::Serialization::Serializer::serialize(def);
        std::string torecv;

        _client->send(tosend, torecv);
        return AL::Serialization::Serializer::deserialize<R>(torecv);
      }
    
      bool initOK;
    protected:
      AL::Transport::Client *_client;
    };

  }
}



#endif  // !AL_MESSAGING_GENERIC_CLIENT_HPP_

/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  AL_MESSAGING_GENERIC_CLIENT_HPP_
# define AL_MESSAGING_GENERIC_CLIENT_HPP_

#include <string>
#include <alcommon-ng/transport/client.hpp>
#include <alcommon-ng/transport/zeromq/zmqclient.hpp>
#include <alcommon-ng/serialization/serializer.hpp>
#include <allog/allog.h>

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

      void call(const T &def, R& result)
      {
        if (! initOK) {
          alserror << "Attempt to use an unitialized client.";
        }
        std::string tosend = AL::Serialization::Serializer::serialize(def);
        std::string torecv;

        _client->send(tosend, torecv);
        // we might have an excetption in the result, even
        // if the call method was void
        AL::Serialization::Serializer::deserialize(torecv, result);
      }

      bool initOK;
    protected:
      AL::Transport::Client *_client;
    };
  }
}

#endif  // AL_MESSAGING_GENERIC_CLIENT_HPP_

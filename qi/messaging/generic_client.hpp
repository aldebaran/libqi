/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_MESSAGING_GENERIC_CLIENT_HPP_
# define QI_MESSAGING_GENERIC_CLIENT_HPP_

#include <string>
#include <qi/transport/client.hpp>
#include <qi/transport/zeromq/zmqclient.hpp>
#include <qi/serialization/serializer.hpp>
#include <allog/allog.h>

namespace qi {
  namespace Messaging {

    template<typename T, typename R>
    class GenericClient {
    public:
      GenericClient() : initOK(false) {}

      bool connect(const std::string &address) {
        try {
          _client = new qi::Transport::ZMQClient(address);
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
        std::string tosend = qi::Serialization::Serializer::serialize(def);
        std::string torecv;

        _client->send(tosend, torecv);
        // we might have an excetption in the result, even
        // if the call method was void
        qi::Serialization::Serializer::deserialize(torecv, result);
      }

      bool initOK;
    protected:
      qi::Transport::Client *_client;
    };
  }
}

#endif  // QI_MESSAGING_GENERIC_CLIENT_HPP_

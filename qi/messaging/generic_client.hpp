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
#include <qi/log.hpp>

namespace qi {
  namespace messaging {

    template<typename T, typename R>
    class GenericClient {
    public:
      GenericClient() : initOK(false) {}

      bool connect(const std::string &address) {
        try {
          _client = new qi::transport::ZMQClient(address);
          initOK = true;
        } catch(const std::exception& e) {
          qisDebug << "GenericClient failed to create client for address \"" << address << "\" Reason: " << e.what() << std::endl;
        }
         return initOK;
      }

      void call(const T &def, R& result)
      {
        if (! initOK) {
          qisError << "Attempt to use an unitialized client." << std::endl;
        }
        std::string tosend = qi::serialization::Serializer::serialize(def);
        std::string torecv;

        _client->send(tosend, torecv);
        // we might have an excetption in the result, even
        // if the call method was void
        qi::serialization::Serializer::deserialize(torecv, result);
      }

      bool initOK;
    protected:
      qi::transport::Client *_client;
    };
  }
}

#endif  // QI_MESSAGING_GENERIC_CLIENT_HPP_

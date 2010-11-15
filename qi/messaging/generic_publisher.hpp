/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_MESSAGING_GENERIC_PUBLISHER_HPP_
# define QI_MESSAGING_GENERIC_PUBLISHER_HPP_

#include <string>
#include <qi/transport/zeromq/zmqpublisher.hpp>
#include <qi/serialization/serializer.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace messaging {

    template<typename T>
    class GenericPublisher {
    public:
      GenericPublisher() : initOK(false) {}

      bool connect(const std::string &address) {
        try {
          _publisher = new qi::transport::ZMQPublisher(address);
          initOK = true;
        } catch(const std::exception& e) {
          qisDebug << "GenericPublisher failed to create publisher for address \"" << address << "\" Reason: " << e.what();
        }
         return initOK;
      }

      void publish(const T& val)
      {
        if (! initOK) {
          qisError << "Attempt to use an unitialized publisher." << std::endl;
          return;
        }
        std::string tosend = qi::serialization::Serializer::serialize(val);
        _publisher->publish(tosend);
      }

      bool initOK;
    protected:
      qi::transport::Publisher* _publisher;
    };
  }
}

#endif  // QI_MESSAGING_GENERIC_PUBLISHER_HPP_

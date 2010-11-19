/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_MESSAGING_GENERIC_PUBLISHER_HPP__
#define   __QI_MESSAGING_GENERIC_PUBLISHER_HPP__

#include <string>
#include <qi/transport/detail/zmq/zmq_publisher.hpp>
#include <qi/serialization/serializer.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace messaging {

    template<typename T>
    class GenericPublisher {
    public:
      GenericPublisher() : isInitialized(false) {}

      bool connect(const std::string &address) {
        try {
          _publisher = new qi::transport::ZMQPublisher(address);
          isInitialized = true;
        } catch(const std::exception& e) {
          qisDebug << "GenericPublisher failed to create publisher for address \"" << address << "\" Reason: " << e.what();
        }
         return isInitialized;
      }

      void publish(const T& val)
      {
        if (! isInitialized) {
          qisError << "Attempt to use an unitialized publisher." << std::endl;
          return;
        }
        qi::serialization::BinarySerializer ser;
        qi::serialization::serialize<T>::write(ser, val);
        _publisher->publish(ser.str());
      }

      bool isInitialized;
    protected:
      qi::transport::Publisher* _publisher;
    };
  }
}

#endif // __QI_MESSAGING_GENERIC_PUBLISHER_HPP__

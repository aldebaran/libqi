#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_MESSAGING_GENERIC_PUBLISHER_HPP_
#define _QI_MESSAGING_GENERIC_PUBLISHER_HPP_

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

      bool bind(const std::string &address) {
        try {
          _publisher = new qi::transport::ZMQPublisher();
          _publisher->bind(address);
          isInitialized = true;
        } catch(const std::exception& e) {
          qisDebug << "GenericPublisher failed to create publisher for address \"" << address << "\" Reason: " << e.what();
        }
         return isInitialized;
      }

      void publish(const T& val)
      {
        if (! isInitialized) {
          qisError << "Attempt to use an uninitialized publisher." << std::endl;
          return;
        }
        qi::serialization::Message ser;
        qi::serialization::serialize<T>::write(ser, val);
        _publisher->publish(ser.str());
      }

      bool isInitialized;
    protected:
      qi::transport::Publisher* _publisher;
    };
  }
}

#endif  // _QI_MESSAGING_GENERIC_PUBLISHER_HPP_

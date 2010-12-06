#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_GENERIC_SUBSCRIBER_HPP_
#define _QI_MESSAGING_GENERIC_SUBSCRIBER_HPP_

#include <string>
#include <qi/transport/detail/zmq/zmq_subscriber.hpp>
#include <qi/serialization/serializer.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace messaging {

    template<typename T>
    class GenericSubscriber : qi::transport::SubscribeHandler {
    public:
      typedef void(*SubscribeHandlerPtr)(const T&);

      GenericSubscriber() : isInitialized(false) {}

      bool connect(const std::string& address) {
        try {
          _subscriber = new qi::transport::ZMQSubscriber();
          _subscriber->connect(address);
          _subscriber->setSubscribeHandler(this);
          isInitialized = true;
        } catch(const std::exception& e) {
          qisDebug << "GenericSubscriber failed to create subscriber for address \"" << address << "\" Reason: " << e.what() << std::endl;
        }
        return isInitialized;
      }

      void subscribe(SubscribeHandlerPtr handler)
      {
        _handler = handler;
        _subscriber->subscribe();
      }

      // called by transport when new data arrives
      void subscribeHandler(qi::transport::Buffer & data) {
        T ret;
        qi::serialization::Message unser(data);
        qi::serialization::serialize<T>::read(unser, ret);
        // call the registered typed-handler
        _handler(ret);
      }

      bool isInitialized;
    protected:
      qi::transport::Subscriber* _subscriber;
      SubscribeHandlerPtr _handler;
    };
  }
}

#endif  // _QI_MESSAGING_GENERIC_SUBSCRIBER_HPP_

/*
** Author(s):
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_MESSAGING_GENERIC_SUBSCRIBER_HPP__
#define   __QI_MESSAGING_GENERIC_SUBSCRIBER_HPP__

#include <string>
#include <qi/transport/detail/zmq/zmq_subscriber.hpp>
#include <qi/serialization/serializer.hpp>
#include <qi/log.hpp>

namespace qi {

    template<typename T>
    class GenericSubscriber : qi::transport::SubscribeHandler {
    public:
      typedef void(*SubscribeHandlerPtr)(const T&);

      GenericSubscriber() : isInitialized(false) {}

      bool connect(const std::string& address) {
        try {
          _subscriber = new qi::transport::ZMQSubscriber(address);
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
      void subscribeHandler(const std::string& data) {
        T ret;
        qi::serialization::BinarySerializer unser(data);
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

#endif // __QI_MESSAGING_GENERIC_SUBSCRIBER_HPP__

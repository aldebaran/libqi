/*
** Author(s):
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_MESSAGING_GENERIC_SUBSCRIBER_HPP_
# define QI_MESSAGING_GENERIC_SUBSCRIBER_HPP_

#include <string>
#include <qi/transport/zeromq/zmqsubscriber.hpp>
#include <qi/serialization/serializer.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace messaging {

    template<typename T>
    class GenericSubscriber : qi::transport::ISubscribeHandler {
    public:
      typedef void(*SubscribeHandlerPtr)(const T&);

      GenericSubscriber() : initOK(false) {}

      bool connect(const std::string& address) {
        try {
          _subscriber = new qi::transport::ZMQSubscriber(address);
          _subscriber->setSubscribeHandler(this);
          initOK = true;
        } catch(const std::exception& e) {
          qisDebug << "GenericSubscriber failed to create subscriber for address \"" << address << "\" Reason: " << e.what() << std::endl;
        }
        return initOK;
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
        unser.read<T>(ret);
        // call the registered typed-handler
        _handler(ret);
      }

      bool initOK;
    protected:
      qi::transport::Subscriber* _subscriber;
      SubscribeHandlerPtr _handler;
    };
  }
}

#endif  // QI_MESSAGING_GENERIC_SUBSCRIBER_HPP_

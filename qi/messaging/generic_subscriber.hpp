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
#include <qi/transport/zeromq/zmqsubscriber.hpp>
#include <qi/serialization/serializer.hpp>
#include <allog/allog.h>

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
          alsdebug << "GenericSubscriber failed to create subscriber for address \"" << address << "\" Reason: " << e.what();
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
        qi::serialization::Serializer::deserialize(data, ret);
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

#endif  // QI_MESSAGING_GENERIC_PUBLISHER_HPP_

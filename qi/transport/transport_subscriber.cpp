/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <string>
#include <qi/transport/transport_context.hpp>
#include <qi/transport/transport_subscriber.hpp>
#include <qi/transport/src/zmq/zmq_subscriber.hpp>
#include <qi/transport/transport_subscribe_handler.hpp>

namespace qi {
  namespace transport {

    TransportSubscriber::TransportSubscriber(TransportContext &ctx)
      : _transportContext(ctx),
        _subscribeHandler(NULL),
        _subscriber(NULL)
    {
    }

    TransportSubscriber::~TransportSubscriber()
    {
      if (_subscriber != NULL) {
        delete _subscriber;
        _subscriber = NULL;
      }
    }

    void TransportSubscriber::setSubscribeHandler(TransportSubscribeHandler* handler) {
      _subscriber->setSubscribeHandler(handler);
    }

    TransportSubscribeHandler* TransportSubscriber::getSubscribeHandler() const {
      return _subscriber->getSubscribeHandler();
    }

    void TransportSubscriber::init() {
      _subscriber = new qi::transport::detail::ZMQSubscriber(_transportContext.getContext<zmq::context_t>(""));
    }

    void TransportSubscriber::connect(const std::string &endpoint) {
      _subscriber->connect(endpoint);
    }

    void TransportSubscriber::subscribe() {
      _subscriber->subscribe();
    }

  }
}

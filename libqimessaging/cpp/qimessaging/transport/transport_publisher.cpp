/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <qimessaging/transport/transport_publisher.hpp>
#include "src/transport/zmq/zmq_publisher.hpp"
#include <qimessaging/transport/transport_context.hpp>
#include <string>
#include <vector>

namespace qi {
  namespace transport {

    TransportPublisher::TransportPublisher(TransportContext &ctx) :
      _transportContext(ctx),
      _publisher(NULL)
    {}

    TransportPublisher::~TransportPublisher()
    {
      if (_publisher != NULL) {
        delete _publisher;
        _publisher = NULL;
      }
    }

    void TransportPublisher::init() {
      _publisher = new qi::transport::detail::ZMQPublisher(_transportContext.getContext<zmq::context_t>(""));
    }

    void TransportPublisher::connect(const std::string& endpoint)
    {
      _publisher->connect(endpoint);
    }

    void TransportPublisher::bind(const std::string& endpoint)
    {
      _publisher->bind(endpoint);
    }

    void TransportPublisher::bind(const std::vector<std::string>& endpoints)
    {
      _publisher->bind(endpoints);
    }

    void TransportPublisher::publish(const std::string& message)
    {
      _publisher->publish(message);
    }

  }
}

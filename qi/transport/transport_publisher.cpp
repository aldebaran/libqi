/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <qi/transport/transport_publisher.hpp>
#include <qi/transport/src/zmq/zmq_publisher.hpp>
#include <qi/transport/transport_context.hpp>
#include <string>
#include <vector>

namespace qi {
  namespace transport {

    TransportPublisher::TransportPublisher(TransportContext *ctx)
    {
      _publisher = new qi::transport::detail::ZMQPublisher();
    }

    TransportPublisher::~TransportPublisher()
    {
      delete _publisher;
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

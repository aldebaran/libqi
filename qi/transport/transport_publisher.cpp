/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <qi/transport/transport_publisher.hpp>
#include <qi/transport/src/zmq/zmq_publisher.hpp>

#include <string>
#include <vector>

namespace qi {
  namespace transport {

    TransportPublisher::TransportPublisher()
    {
      _publisher = new qi::transport::detail::ZMQPublisher();
    }

    TransportPublisher::~TransportPublisher()
    {
      delete _publisher;
    }

    void TransportPublisher::connect(const std::string& serverAddresses)
    {
      _publisher->connect(serverAddresses);
    }

    void TransportPublisher::bind(const std::string& serverAddress)
    {
      _publisher->bind(serverAddress);
    }

    void TransportPublisher::bind(const std::vector<std::string>& serverAddresses)
    {
      _publisher->bind(serverAddresses);
    }

    void TransportPublisher::publish(const std::string& tosend)
    {
      _publisher->publish(tosend);
    }

  }
}

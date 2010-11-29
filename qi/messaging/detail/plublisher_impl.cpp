/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/publisher_impl.hpp>
#include <qi/transport/detail/zmq/zmq_publisher.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace detail {

    PublisherImpl::PublisherImpl() : _isInitialized(false) {}

    bool PublisherImpl::bind(const std::string& address) {
      try {
        _publisher = new qi::transport::ZMQPublisher();
        _publisher->bind(address);
        _isInitialized = true;
      } catch(const std::exception& e) {
        qisDebug << "Publisher failed to create publisher for address \"" << address << "\" Reason: " << e.what();
      }
      return _isInitialized;
    }

    void PublisherImpl::publish(const std::string& data)
    {
      if (! _isInitialized) {
        qisError << "Attempt to use an uninitialized publisher." << std::endl;
        return;
      }
      _publisher->publish(data);
    }

    PublisherImpl::~PublisherImpl() {
      delete _publisher;
      _publisher = NULL;
    }
  }
}

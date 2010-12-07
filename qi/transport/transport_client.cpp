/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/transport_client.hpp>

#include <qi/transport/buffer.hpp>
#include <qi/transport/src/client_backend.hpp>
#include <qi/transport/src/zmq/zmq_client_impl.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace transport {

    TransportClient::TransportClient()
      : _isInitialized(false)
    {
    }

    bool TransportClient::connect(const std::string &address) {
      try {
        _client = new qi::transport::detail::ZMQClientBackend(address);
        _isInitialized = true;
      } catch(const std::exception& e) {
        qisDebug << "GenericClient failed to create client for address \""
            << address << "\" Reason: " << e.what() << std::endl;
        _isInitialized = false;
      }
      return _isInitialized;
    }

    void TransportClient::send(const qi::transport::Buffer &def, qi::transport::Buffer &result)
    {
      if (!_isInitialized) {
        qisError << "Attempt to use an uninitialized client." << std::endl;
        }
      _client->send(def, result);
    }

  }
}

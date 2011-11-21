/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qimessaging/transport/transport_client.hpp>
#include <qimessaging/transport/transport_context.hpp>
#include <qimessaging/transport/buffer.hpp>
#include "src/transport/client_backend.hpp"
#include "src/transport/zmq/zmq_client_backend.hpp"
#include "src/transport/ssh/ssh_client_backend.hpp"
#include <qi/log.hpp>

namespace qi {
  namespace transport {

    TransportClient::TransportClient(TransportContext &context)
      : _transportContext(context),
        _isInitialized(false),
        _client(NULL)
    {
    }

    TransportClient::~TransportClient() {
      if (_client != NULL) {
        delete(_client);
        _client = NULL;
      }
    }

    bool TransportClient::connect(const std::string &endpoint) {
      try {
        _client = new qi::transport::detail::ZMQClientBackend(endpoint, _transportContext.getContext<zmq::context_t>(endpoint));
        _isInitialized = true;
      } catch(const std::exception& e) {
        qiLogDebug("qimessaging") << "GenericClient failed to create client for address \""
            << endpoint << "\" Reason: " << e.what() << std::endl;
        _isInitialized = false;
      }
      return _isInitialized;
    }

    void TransportClient::send(const qi::transport::Buffer &request, qi::transport::Buffer &reply)
    {
      if (!_isInitialized) {
        qiLogError("qimessaging") << "Attempt to use an uninitialized client." << std::endl;
        return;
      }
      _client->send(request, reply);
    }

  }
}

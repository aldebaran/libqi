/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <string>
#include <qi/log.hpp>
#include <qimessaging/transport/transport_forwarder.hpp>
#include <qimessaging/transport/transport_context.hpp>
#include "src/transport/forwarder_backend.hpp"
#include "src/transport/zmq/zmq_forwarder_backend.hpp"

namespace qi {
  namespace transport {

    TransportForwarder::TransportForwarder(const TransportForwarder& rhs) :
      _isInitialized(rhs._isInitialized),
      _transportContext(rhs._transportContext),
      _backend(rhs._backend) {}

    TransportForwarder::TransportForwarder(TransportContext &context)
      : _isInitialized(false),
        _transportContext(context),
        _backend(NULL)
    {
    }

    TransportForwarder::~TransportForwarder() {
      if (_backend != NULL) {
        delete(_backend);
        _backend = NULL;
      }
    }

    void TransportForwarder::bind(
      const std::vector<std::string>& inEndpoints,
      const std::vector<std::string>& outEndpoints)
    {
      try {
        _backend = new detail::ZMQForwarderBackend(
          inEndpoints,
          outEndpoints,
          _transportContext.getContext<zmq::context_t>(inEndpoints[0])
          );
        _backend->bind();

        _isInitialized = true;
      } catch(const zmq::error_t& e) {
        qisError << "Failed to bind the transport forwarder. Reason: "
          << e.what() << std::endl;
        throw(e);
      }
    }

    void TransportForwarder::run() {
      _backend->run();
    }

    bool TransportForwarder::isInitialized() {
      return _isInitialized;
    }
  }
}


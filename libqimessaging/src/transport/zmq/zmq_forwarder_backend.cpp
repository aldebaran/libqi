/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/transport/zmq/zmq_forwarder_backend.hpp"

namespace qi {
  namespace transport {
    namespace detail {

      ZMQForwarderBackend::ZMQForwarderBackend(
        const std::vector<std::string>& inAddresses,
        const std::vector<std::string>& outAddresses,
        zmq::context_t &context) :
          _context(context),
          _in_socket(_context, ZMQ_SUB),
          _out_socket(_context, ZMQ_PUB),
           ForwarderBackend(inAddresses, outAddresses)
      {
        _in_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
      }

      ZMQForwarderBackend::~ZMQForwarderBackend() {}

      void ZMQForwarderBackend::bind() {
        std::vector<std::string>::const_iterator it;
        for(it = _inAddresses.begin(); it != _inAddresses.end(); ++it) {
          _in_socket.bind( (*it).c_str());
        }
        for(it = _outAddresses.begin(); it != _outAddresses.end(); ++it) {
          _out_socket.bind( (*it).c_str());
        }
      }

      void ZMQForwarderBackend::run()
      {
        // FIXME: how can this be torn down?
        zmq::device(ZMQ_FORWARDER, _in_socket, _out_socket);
      }
    }
  }
}


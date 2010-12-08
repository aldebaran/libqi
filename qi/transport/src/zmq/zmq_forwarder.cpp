/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/src/zmq/zmq_forwarder.hpp>

namespace qi {
  namespace transport {
    namespace detail {

    ZMQForwarder::ZMQForwarder(zmq::context_t &context)
      : _context(context),
      _in_socket(_context, ZMQ_SUB),
      _out_socket(_context, ZMQ_PUB)
    {
      _in_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    }

    ZMQForwarder::~ZMQForwarder() {}

    void ZMQForwarder::run(const std::string& inEndpoint, const std::string& outEndpoint) {
      std::vector<std::string> in;
      std::vector<std::string> out;
      in.push_back(inEndpoint);
      out.push_back(outEndpoint);
      run(in, out);
    }


    void ZMQForwarder::run(const std::vector<std::string>& inEndpoints, const std::vector<std::string>& outEndpoints)
    {
      std::vector<std::string>::const_iterator it;
      for(it = inEndpoints.begin(); it != inEndpoints.end(); ++it) {
        _in_socket.bind( (*it).c_str());
      }
      for(it = outEndpoints.begin(); it != outEndpoints.end(); ++it) {
        _out_socket.bind( (*it).c_str());
      }
      // FIXME: how can this be torn down?
      zmq::device(ZMQ_FORWARDER, _in_socket, _out_socket);
    }
    }
  }
}


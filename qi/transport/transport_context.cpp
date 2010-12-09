/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/transport_context.hpp>
#include <zmq.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace transport {

    TransportContext::TransportContext(const TransportContext& rhs) :
      _ctx(rhs._ctx) {}

    TransportContext::TransportContext()
    {
      _ctx = static_cast<void *>(new zmq::context_t(1));
    }

    TransportContext::~TransportContext()
    {
      delete static_cast<zmq::context_t *>(_ctx);
    }

  }
}

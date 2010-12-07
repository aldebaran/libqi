/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/src/zmq/zmq_publisher.hpp>
#include <qi/perf/sleep.hpp>

namespace qi {
  namespace transport {
    namespace detail {

      ZMQPublisher::ZMQPublisher():
    _context(boost::shared_ptr<zmq::context_t> (new zmq::context_t(1))),
      _socket(*_context.get(), ZMQ_PUB)
    {}

    ZMQPublisher::ZMQPublisher(boost::shared_ptr<zmq::context_t> context)
      : _context(context),
      _socket(*_context.get(), ZMQ_PUB)
    {}

    ZMQPublisher::~ZMQPublisher() {}

    boost::shared_ptr<zmq::context_t> ZMQPublisher::getContext() const {
      return _context;
    }

    void ZMQPublisher::connect(const std::string& publishEndpoint)
    {
      // throws if already bound
      _socket.connect(publishEndpoint.c_str());
      // we can't allow publishing until the socket is warm
      // we might be able to detect this in publish instead of sleeping here
      // FIXME: push this responsibility to the user
      sleep(1);
    }

    void ZMQPublisher::bind(const std::string& publishEndpoint)
    {
      std::vector<std::string> v;
      v.push_back(publishEndpoint);
      bind(v);
    }

    void ZMQPublisher::bind(const std::vector<std::string> &publishEndpoints)
    {
      // throws if already bound
      std::vector<std::string>::const_iterator it = publishEndpoints.begin();
      for (; it != publishEndpoints.end(); it++) {
        _socket.bind((*it).c_str());
      }
      // we can't allow publishing until the socket is warm
      // we might be able to detect this in publish instead of sleeping here
      // FIXME: push this responsibility to the user
      sleep(1);
    }

    void ZMQPublisher::publish(const std::string &toSend)
    {
      zmq::message_t msg(toSend.size());
      memcpy(msg.data(), toSend.data(), toSend.size());
      _socket.send(msg);
    }

    }
  }
}


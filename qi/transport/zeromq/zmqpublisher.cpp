/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/zeromq/zmqpublisher.hpp>
#include <qi/perf/sleep.hpp>

namespace qi {
  namespace transport {

    /// <summary> Constructor. </summary>
    /// <param name="publishAddress"> The server address. </param>
    ZMQPublisher::ZMQPublisher(const std::string &publishAddress)
      : Publisher(publishAddress),
      _context(boost::shared_ptr<zmq::context_t> (new zmq::context_t(1))),
      _socket(*_context.get(), ZMQ_PUB)
    {
      bind();
    }

    /// <summary> Constructor. </summary>
    /// <param name="context"> An existing zmq context. </param>
    /// <param name="publishAddress"> The server address. </param>
    ZMQPublisher::ZMQPublisher(boost::shared_ptr<zmq::context_t> context, const std::string &publishAddress)
      : Publisher(publishAddress),
      _context(context),
      _socket(*_context.get(), ZMQ_PUB)
    {
      bind();
    }

    ZMQPublisher::~ZMQPublisher() {}

    boost::shared_ptr<zmq::context_t> ZMQPublisher::getContext() const {
      return _context;
    }

    /// <summary> Binds to the publisher </summary>
    void ZMQPublisher::bind()
    {
      // throws if already bound
      _socket.bind(_publishAddress.c_str());
      // we can't allow publishing until the socket is warm
      // we might be able to detect this in publish instead of sleeping here
      // FIXME: push this responsibility to the user
      sleep(1);
    }

    /// <summary> Publishes. </summary>
    /// <param name="tosend"> The data to send. </param>
    void ZMQPublisher::publish(const std::string &tosend)
    {
      zmq::message_t msg(tosend.size());
      memcpy(msg.data(), tosend.data(), tosend.size());
      bool rc = _socket.send(msg);
    }
  }
}


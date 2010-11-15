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
      context(1),
      socket(context, ZMQ_PUB)
    {
      // Release socket immediately
      // int lingerMilliseconds = 0;
      // socket.setsockopt(ZMQ_LINGER, &lingerMilliseconds, sizeof(int));

      bind();
    }

    ZMQPublisher::~ZMQPublisher() {}

    /// <summary> Binds to the publisher </summary>
    void ZMQPublisher::bind()
    {
      // throws if already bound
      socket.bind(_publishAddress.c_str());
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
      bool rc = socket.send(msg);
    }
  }
}


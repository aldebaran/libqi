/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/zeromq/zmqpublisher.hpp>

namespace qi {
  namespace transport {

    /// <summary> Constructor. </summary>
    /// <param name="publishAddress"> The server address. </param>
    ZMQPublisher::ZMQPublisher(const std::string &publishAddress)
      : Publisher(publishAddress),
      context(1),
      socket(context, ZMQ_PUB)
    {
      bind();
    }

    /// <summary> Binds to the publisher </summary>
    void ZMQPublisher::bind()
    {
      socket.bind(_publishAddress.c_str());
    }

    /// <summary> Publishes. </summary>
    /// <param name="tosend"> The data to send. </param>
    void ZMQPublisher::publish(const std::string &tosend)
    {
      zmq::message_t msg(tosend.size());
      memcpy(msg.data(), tosend.data(), tosend.size());
      socket.send(msg);
    }
  }
}


/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/zeromq/zmqsubscriber.hpp>
#include <iostream>

namespace qi {
  namespace transport {

    /// <summary> Constructor. </summary>
    /// <param name="publishAddress"> The publishing address. </param>
    ZMQSubscriber::ZMQSubscriber(const std::string &publishAddress)
      : Subscriber(publishAddress),
      context(1),
      socket(context, ZMQ_SUB)
    {
      socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
      connect();
    }

    /// <summary> Connects to the publisher </summary>
    void ZMQSubscriber::connect()
    {
      socket.connect(_publishAddress.c_str());
    }

    /// <summary> Subscribe. </summary>
    void ZMQSubscriber::subscribe()
    {
      // todo make a thread here
      while(true)
      {
        zmq::message_t msg;
        socket.recv (&msg);
        std::cout << "Subscriber received" << std::endl;
        std::string data;
        data.assign((char *)msg.data(), msg.size());
        (this->getSubscribeHandler())->subscribeHandler(data);
      }
    }
  }
}


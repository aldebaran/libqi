/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/zeromq/zmqsubscriber.hpp>
#include <qi/perf/sleep.hpp>

namespace qi {
  namespace transport {
    /// <summary> Constructor. </summary>
    /// <param name="publishAddress"> The publishing address. </param>
    ZMQSubscriber::ZMQSubscriber(const std::string &publishAddress)
      : Subscriber(publishAddress),
      context(1),
      socket(context, ZMQ_SUB),
      control(context, ZMQ_PUB)
    {
      // Use no subscribe filter
      socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

      // Release socket immediately when closing
      //int lingerMilliseconds = 0;
      //socket.setsockopt(ZMQ_LINGER, &lingerMilliseconds, sizeof(int));

      connect();
    }

    ZMQSubscriber::~ZMQSubscriber() {
      // Discussion: We have a blocking receive in
      // progress. This is rather difficult to interupt.
      // We have various options:
      // 1) close the socket ( doesn't work ... error in receive)
      // 2) terminate the context ( meant to work , but...)
      // 3) use poll with short timeout and signal an exit
      // 4) send a poison message to the receiver.

      // Poison message: Unfortunately, it uses an extra socket
      // and probably requires a unique socket name, otherwise
      // the second subscriber will fail the bind.
      zmq::message_t msg(7);
      memcpy(msg.data(), "!KILL!", 7);
      //  Send kill signal to receive socket
      control.send(msg);
      sleep(1);
    }

    /// <summary> Connects to the publisher </summary>
    void ZMQSubscriber::connect()
    {
      control.bind("inproc://control");
      socket.connect(_publishAddress.c_str());
      socket.connect("inproc://control");
      // we can't allow continuation until the socket is warm
      // FIXME: push this responsibility to the user
      //sleep(1);
    }

    void ZMQSubscriber::receive()
    {
      bool ok;
      try {
        while(true)
        {
          zmq::message_t msg;
          ok = socket.recv (&msg);
          if (!ok) {
            std::cout << "ZMQSubscriber::recv failed.";
            return;
          }
          std::string data;
          data.assign((char *)msg.data(), msg.size());
          if (strcmp((char *)msg.data(), "!KILL!")==0) {
            std::cout << "ZMQSubscriber:: received kill message" << std::endl;
            return;
          }
          // No way to notice that the subscriber handler has
          // been deallocated. If it has, this will segfault
          this->getSubscribeHandler()->subscribeHandler(data);
        }
      } catch(const std::exception&) {
        // don't print what() because it may be invalid during destruction
        std::cout << "ZMQSubscriber::recv exception " << std::endl;
      } catch(...) {
        std::cout << "ZMQSubscriber::deallocated? " << std::endl;
      }
    }

    /// <summary> Subscribe. </summary>
    void ZMQSubscriber::subscribe()
    {
      receiveThread = boost::thread(&ZMQSubscriber::receive, this);
    }
  }
}


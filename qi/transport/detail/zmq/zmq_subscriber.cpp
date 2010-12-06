/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/detail/zmq/zmq_subscriber.hpp>
#include <qi/perf/sleep.hpp>
#include <iostream>

namespace qi {
  namespace transport {

    /// <summary> Constructor. </summary>
    ZMQSubscriber::ZMQSubscriber()
      :
      _isClosing(false),
      _context(boost::shared_ptr<zmq::context_t>(new zmq::context_t(1))),
      _socket(*_context.get(), ZMQ_SUB),
      _control(*_context.get(), ZMQ_PUB)
    {
      // Use no subscribe filter
      _socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    }


    /// <summary> Constructor. </summary>
    /// <param name="publishAddress"> The publishing address. </param>
    ZMQSubscriber::ZMQSubscriber(boost::shared_ptr<zmq::context_t> context)
      :
      _isClosing(false),
      _context(context),
      _socket(*_context.get(), ZMQ_SUB),
      _control(*_context.get(), ZMQ_PUB)
    {
      // Use no subscribe filter
      _socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    }

    ZMQSubscriber::~ZMQSubscriber() {
      _isClosing = true;
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
      _control.send(msg);
      // if the socket tries to close before being killed,
      // we will deadlock
      sleep(1.0);
    }

    /// <summary> Connects to the publisher </summary>
    void ZMQSubscriber::connect(const std::string &publishAddress)
    {
      // FIXME:
      // this will not work if sharing the same context,
      // as another subscriber... need uuid
      _control.bind("inproc://control");
      _socket.connect(publishAddress.c_str());
      _socket.connect("inproc://control");
    }

    void ZMQSubscriber::receive()
    {
      bool ok;
      try {
        while(!_isClosing)
        {
          zmq::message_t msg;
          ok = _socket.recv (&msg);
          if (!ok) {
            std::cout << "ZMQSubscriber::recv failed." << std::endl;
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
          //          // getSubscribeHandler
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
      _receiveThread = boost::thread(&ZMQSubscriber::receive, this);
    }
  }
}


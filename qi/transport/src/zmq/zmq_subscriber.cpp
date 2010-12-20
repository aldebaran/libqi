/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/src/zmq/zmq_subscriber.hpp>
#include <qi/perf/sleep.hpp>
#include <qi/exceptions/exceptions.hpp>
#include <iostream>

namespace qi {
  namespace transport {
    namespace detail {

      ZMQSubscriber::ZMQSubscriber(zmq::context_t &context)
        : _isClosing(false),
          _context(context),
          _socket(_context, ZMQ_SUB),
          _poller(_socket)
      {

        int linger = 0;
#ifdef ZMQ_LINGER
        _socket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
#endif
        // Use no subscribe filter
        _socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
      }

      ZMQSubscriber::~ZMQSubscriber() {
        _isClosing = true;
        sleep(1.0);
      }

      void ZMQSubscriber::connect(const std::string &publishAddress)
      {
        _socket.connect(publishAddress.c_str());
      }

      void ZMQSubscriber::receive()
      {
        try {
          while(!_isClosing)
          {
            zmq::message_t msg;
            _poller.recv(&msg);
            std::string data((char *)msg.data(), msg.size());

            // No way to notice that the subscriber handler has
            // been deallocated. If it has, this will segfault
            this->getSubscribeHandler()->subscribeHandler(data);
          }
        } catch(const qi::transport::Exception& e) {
          // don't print what() because it may be invalid during destruction
          std::cout << "ZMQSubscriber::recv exception. Reason: " << e.what() << std::endl;
        }
      }

      void ZMQSubscriber::subscribe()
      {
        _receiveThread = boost::thread(&ZMQSubscriber::receive, this);
      }
    }
  }
}


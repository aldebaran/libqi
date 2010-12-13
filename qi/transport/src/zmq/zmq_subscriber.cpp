/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/src/zmq/zmq_subscriber.hpp>
#include <qi/perf/sleep.hpp>
#include <iostream>

namespace qi {
  namespace transport {
    namespace detail {

      ZMQSubscriber::ZMQSubscriber(zmq::context_t &context)
        : _isClosing(false),
          _context(context),
          _socket(_context, ZMQ_SUB)
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

      bool ZMQSubscriber::poll(long timeout) {
        int             rc = 0;
        zmq_pollitem_t  items[1];

        items[0].socket  = _socket;
        items[0].fd      = 0;
        items[0].events  = ZMQ_POLLIN;
        items[0].revents = 0;

        // unfortunately there is an assert in getsockopt
        rc = zmq::poll(&items[0], 1, timeout);
        assert(rc >= 0);
        return (items[0].revents & ZMQ_POLLIN);
      }

      void ZMQSubscriber::receive()
      {
        bool ok;
        try {
          while(!_isClosing)
          {
            zmq::message_t msg;
            bool haveMessage = false;
            while (!haveMessage) {
              haveMessage = poll(1000*1000);
              if (_isClosing)
                return;
            }
            ok = _socket.recv(&msg);
            if (!ok) {
              std::cout << "ZMQSubscriber::recv failed." << std::endl;
              return;
            }
            std::string data((char *)msg.data(), msg.size());
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

      void ZMQSubscriber::subscribe()
      {
        _receiveThread = boost::thread(&ZMQSubscriber::receive, this);
      }
    }
  }
}


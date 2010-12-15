/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/src/zmq/zmq_poll_client.hpp>
#include <qi/exceptions/exceptions.hpp>
#include <iostream>
#include <qi/log.hpp>

namespace qi {
  namespace transport {
    namespace detail {
      ZMQPollClient::ZMQPollClient(zmq::socket_t &socket)
        : _zsocket(socket),
          _first_time(true)
      {
        _items[0].socket  = _zsocket;
        _items[0].fd      = 0;
        _items[0].events  = ZMQ_POLLIN;
        _items[0].revents = 0;
      }

      //return -1 on error, 0 otherwise
      int ZMQPollClient::pollRecv(long timeout) {
        int             rc = 0;

        rc = zmq::poll(_items, 1, timeout);
        //if (rc <= 0) { // less debug when ok
        //  qisDebug << "ZMQPollClient rc:" << rc << " _items[0].revents: " << _items[0].revents << " timeout: " << timeout << std::endl;
        //}
        if ((rc <= 0) || (!(_items[0].revents & ZMQ_POLLIN)))
          return -1;
        return 0;
      }

      void ZMQPollClient::recv(zmq::message_t *msg, long usTimeout)
      {
        if (_first_time) {
          int rc = 0;
          long elapsed = 0;
          _first_time = false;
          do {
            rc = pollRecv(10 * 1000); // 10 ms
            elapsed += 1;
          } while (rc < 0 && elapsed < usTimeout);
          if (rc < 0)
            throw qi::transport::Exception("No response received.");
        }
        else
        {
           if (pollRecv(usTimeout) < 0)
            throw qi::transport::Exception("No response received.");
        }
        _zsocket.recv(msg);
      }
    }
  }
}


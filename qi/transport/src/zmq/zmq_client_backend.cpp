/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/src/zmq/zmq_client_backend.hpp>
#include <qi/exceptions/exceptions.hpp>
#include <iostream>

namespace qi {
  namespace transport {
    namespace detail {
      /// <summary> Constructor. </summary>
      /// <param name="serverAddress"> The server address. </param>
      ZMQClientBackend::ZMQClientBackend(const std::string &serverAddress, zmq::context_t &context)
        : ClientBackend(serverAddress),
          _zcontext(context),
          _zsocket(context, ZMQ_REQ)
      {
        int linger = 0;
#ifdef ZMQ_LINGER
        _zsocket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
#endif
        connect();
      }

      /// <summary> Connects to the server </summary>
      void ZMQClientBackend::connect()
      {
        _zsocket.connect(_serverAddress.c_str());
        //TODO: check that the connection is OK
        //sleep(1);
      }

      void ZMQClientBackend::pollRecv(long timeout) {
        int             rc = 0;
        zmq_pollitem_t  items[1];

        items[0].socket  = _zsocket;
        items[0].fd      = 0;
        items[0].events  = ZMQ_POLLIN;
        items[0].revents = 0;

        rc = zmq::poll(items, 1, timeout);
        std::cout << "timeout:" << timeout << std::endl;
        std::cout << "Rc:" << rc << std::endl;
        std::cout << "PollIn:" << (items[0].revents) << std::endl;
        if ((rc <= 0) || (!(items[0].revents & ZMQ_POLLIN)))
          throw qi::transport::Exception("no response");
      }

      /// <summary> Sends. </summary>
      /// <param name="tosend"> The data to send. </param>
      /// <param name="result"> [in,out] The result. </param>
      void ZMQClientBackend::send(const std::string &tosend, std::string &result)
      {
        // TODO optimise this
        // Could we copy from the serialized stream without calling
        // stream.str() before sending to this method?
        //TODO: could we avoid more copy?
        zmq::message_t msg(tosend.size());
        memcpy(msg.data(), tosend.data(), tosend.size());
        _zsocket.send(msg);

        //we leave the possibility to timeout, pollRecv will throw and avoid the call to recv
        //pollRecv(1000 * 1000 * 1000);
        _zsocket.recv(&msg);

        // TODO optimize this
        // boost could serialize from msg.data() and size,
        // without making a string
        result.assign((char *)msg.data(), msg.size());
      }
    }
  }
}


/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
*/

#include <qi/transport/zeromq/zmqclientimpl.hpp>
//#include <qi/transport/shm/client/result_handler.hpp>
//#include <sstream>

namespace qi {
  namespace transport {
    namespace detail {
      /// <summary> Constructor. </summary>
      /// <param name="serverAddress"> The server address. </param>
      ZMQClientImpl::ZMQClientImpl(const std::string &serverAddress)
        : ClientImpl(serverAddress),
        context(1),
        socket(context, ZMQ_REQ)
      {
        connect();
      }

      /// <summary> Connects to the server </summary>
      void ZMQClientImpl::connect()
      {
        socket.connect(_serverAddress.c_str());
      }

      /// <summary> Sends. </summary>
      /// <param name="tosend"> The data to send. </param>
      /// <param name="result"> [in,out] The result. </param>
      void ZMQClientImpl::send(const std::string &tosend, std::string &result)
      {
        // TODO optimise this
        // Could we copy from the serialized stream without calling
        // stream.str() before sending to this method?
        //TODO: could we avoid more copy?
        zmq::message_t msg(tosend.size());
        memcpy(msg.data(), tosend.data(), tosend.size());
        socket.send(msg);
        socket.recv(&msg);
        // TODO optimize this
        // boost could serialize from msg.data() and size,
        // without making a string
        result.assign((char *)msg.data(), msg.size());
      }
    }
  }
}


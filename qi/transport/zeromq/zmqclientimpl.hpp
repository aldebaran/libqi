/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_TRANSPORT_ZEROMQ_ZMQCLIENTIMPL_HPP_
# define QI_TRANSPORT_ZEROMQ_ZMQCLIENTIMPL_HPP_

# include <qi/transport/buffer.hpp>
# include <qi/transport/detail/clientimpl.hpp>
# include <zmq.hpp>

namespace qi {
  namespace transport {

    class ZMQClientImpl : public qi::transport::detail::ClientImpl<qi::transport::Buffer> {
    public:
      //ZMQClient(const std::string &servername, ResultHandler *resultHandler);

      /// <summary>
      /// Creates a ZMQClient of a server
      /// </summary>
      /// <param name="serverAddress">
      /// The protocol-qualified address of the server
      /// e.g. ipc:///tmp/naoqi/paf
      //. or tcp://127.0.0.1:5555
      /// </param>
      ZMQClientImpl(const std::string &servername);

      virtual void send(const std::string &tosend, std::string &result);

    protected:
      void connect();

    protected:
      zmq::context_t context;
      zmq::socket_t  socket;
    };
  }
}

#endif  // QI_TRANSPORT_ZMQCLIENT_HPP_

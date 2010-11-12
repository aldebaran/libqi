/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_TRANSPORT_ZMQPUBLISHER_HPP_
# define QI_TRANSPORT_ZMQPUBLISHER_HPP_

# include <qi/transport/publisher.hpp>
# include <zmq.hpp>

namespace qi {
  namespace transport {

    class ZMQPublisher : public Publisher {
    public:
      /// <summary>
      /// Creates a ZMQPublisher
      /// </summary>
      /// <param name="publisherAddress">
      /// The protocol-qualified address of the publisher
      /// e.g. ipc:///tmp/naoqi/paf
      //. or tcp://127.0.0.1:5555
      /// </param>
      ZMQPublisher(const std::string &publisherAddress);

      virtual ~ZMQPublisher();

      virtual void publish(const std::string &tosend);

    protected:
      void bind();

    protected:
      zmq::context_t context;
      zmq::socket_t  socket;
    };
  }
}

#endif  // QI_TRANSPORT_ZMQCLIENT_HPP_

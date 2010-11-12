/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_TRANSPORT_ZMQSUBSCRIBER_HPP_
# define QI_TRANSPORT_ZMQSUBSCRIBER_HPP_

# include <qi/transport/subscriber.hpp>
# include <zmq.hpp>

namespace qi {
  namespace transport {

    class ZMQSubscriber : public Subscriber {
    public:
      /// <summary>
      /// Creates a ZMQSubscriber
      /// </summary>
      /// <param name="serverAddress">
      /// The protocol-qualified address of the publisher
      /// e.g. ipc:///tmp/naoqi/paf
      //. or tcp://127.0.0.1:5555
      /// </param>
      ZMQSubscriber(const std::string &servername);

      virtual ~ZMQSubscriber();

      virtual void subscribe();

    protected:
      void connect();

    protected:
      zmq::context_t context;
      zmq::socket_t  socket;
    };
  }
}

#endif  // QI_TRANSPORT_ZMQSUBSCRIBER_HPP_

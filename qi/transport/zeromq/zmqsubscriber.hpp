/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_TRANSPORT_ZMQSUBSCRIBER_HPP_
# define QI_TRANSPORT_ZMQSUBSCRIBER_HPP_

# include <qi/transport/subscriber.hpp>
# include <boost/thread.hpp>
# include <zmq.hpp>
# include <boost/shared_ptr.hpp>

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
      ZMQSubscriber(const std::string &serverAddress);

      /// <summary>
      /// Creates a ZMQSubscriber
      /// </summary>
      /// <param name="serverAddress">
      /// The protocol-qualified address of the publisher
      /// e.g. ipc:///tmp/naoqi/paf
      //. or tcp://127.0.0.1:5555
      /// </param>
      ZMQSubscriber(boost::shared_ptr<zmq::context_t>, const std::string &serverAddress);

      virtual ~ZMQSubscriber();

      virtual void subscribe();

    protected:
      void connect();
      void receive();

    protected:
      bool _isClosing;
      boost::shared_ptr<zmq::context_t> _context;
      zmq::socket_t  _socket;
      zmq::socket_t  _control;
      boost::thread  _receiveThread;
    };
  }
}

#endif  // QI_TRANSPORT_ZMQSUBSCRIBER_HPP_

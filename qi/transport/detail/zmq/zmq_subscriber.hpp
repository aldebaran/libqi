/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SUBSCRIBER_HPP__
#define   __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SUBSCRIBER_HPP__

# include <qi/transport/subscriber.hpp>
# include <boost/thread.hpp>
# include <zmq.hpp>
# include <boost/shared_ptr.hpp>

namespace qi {
  namespace transport {

    class ZMQSubscriber : public Subscriber {
    public:
      /// <summary>Creates a ZMQSubscriber</summary>
      /// <param name="publishAddress">
      /// The protocol-qualified address of the publisher
      /// e.g. ipc:///tmp/naoqi/paf
      //. or tcp://127.0.0.1:5555
      /// </param>
      ZMQSubscriber(const std::string &publishAddress);

      /// <summary> Creates a ZMQSubscriber </summary>
      /// <param name="context">An existing zmq context</param>
      /// <param name="publishAddress">
      /// The protocol-qualified address of the publisher
      /// e.g. ipc:///tmp/naoqi/paf
      //. or tcp://127.0.0.1:5555
      /// </param>
      ZMQSubscriber(boost::shared_ptr<zmq::context_t>, const std::string &publishAddress);

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

#endif // __QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SUBSCRIBER_HPP__

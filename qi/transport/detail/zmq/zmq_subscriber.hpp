#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SUBSCRIBER_HPP_
#define _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SUBSCRIBER_HPP_

# include <qi/transport/transport_subscriber.hpp>
# include <boost/thread.hpp>
# include <zmq.hpp>
# include <boost/shared_ptr.hpp>

namespace qi {
  namespace transport {

    class ZMQSubscriber : public TransportSubscriber {
    public:
      /// <summary>Creates a ZMQSubscriber</summary>
      ZMQSubscriber();

      /// <summary> Creates a ZMQSubscriber </summary>
      /// <param name="context">An existing zmq context</param>
      ZMQSubscriber(boost::shared_ptr<zmq::context_t>);

      virtual ~ZMQSubscriber();

      virtual void subscribe();

      /// <param name="publishAddress">
      /// The protocol-qualified address of the publisher
      /// e.g. ipc:///tmp/naoqi/paf
      //. or tcp://127.0.0.1:5555
      /// </param>
      void connect(const std::string& subscribeAddress);

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

#endif  // _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_SUBSCRIBER_HPP_

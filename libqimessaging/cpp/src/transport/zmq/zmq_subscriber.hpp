#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_ZMQ_ZMQ_SUBSCRIBER_HPP_
#define _QI_TRANSPORT_SRC_ZMQ_ZMQ_SUBSCRIBER_HPP_

#include "src/transport/subscriber_backend.hpp"
#include "src/transport/zmq/zmq_poll_client.hpp"
#include <boost/thread.hpp>
#include <zmq.hpp>
#include <boost/shared_ptr.hpp>


namespace qi {
  namespace transport {
    namespace detail {
      class ZMQSubscriber : public SubscriberBackend {
      public:

        /// <summary> Creates a ZMQSubscriber </summary>
        /// <param name="context">An existing zmq context</param>
        ZMQSubscriber(zmq::context_t &context);

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
        bool            _isClosing;
        zmq::context_t &_context;
        zmq::socket_t   _socket;
        ZMQPollClient   _poller;
        boost::thread   _receiveThread;
      };
    }
  }
}

#endif  // _QI_TRANSPORT_SRC_ZMQ_ZMQ_SUBSCRIBER_HPP_

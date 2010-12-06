#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_PUBLISHER_HPP_
#define _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_PUBLISHER_HPP_

# include <qi/transport/publisher.hpp>
# include <zmq.hpp>
# include <boost/shared_ptr.hpp>

namespace qi {
  namespace transport {

    class ZMQPublisher : public Publisher {
    public:
      /// <summary> Default Constructor. </summary>
      ZMQPublisher();

      /// <summary> Constructor that allows an existing zmq context to be used </summary>
      /// <param name="context">An existing zmq context</param>
      ZMQPublisher(boost::shared_ptr<zmq::context_t> context);

      /// <summary> Destructor </summary>
      virtual ~ZMQPublisher();

      /// <summary> Connects to a forwarder </summary>
      /// <param name="publishEndpoint"> The forwarder's publish endpoint </param>
      void connect(const std::string& publishEndpoint);

      /// <summary> Binds to the publisher </summary>
      /// <param name="publishEndpoint"> The endpoint to bind to </param>
      void bind(const std::string& publishEndpoint);

      /// <summary> Binds to the publisher </summary>
      /// <param name="publishEndpoints"> The endpoints to bind to </param>
      void bind(const std::vector<std::string> &publishEndpoints);

      boost::shared_ptr<zmq::context_t> getContext() const;

      /// <summary> Publishes. </summary>
      /// <param name="toSend"> The data to send. </param>
      virtual void publish(const std::string &tosend);

    protected:
      boost::shared_ptr<zmq::context_t> _context;
      zmq::socket_t                     _socket;
    };
  }
}

#endif  // _QI_TRANSPORT_DETAIL_ZMQ_ZMQ_PUBLISHER_HPP_

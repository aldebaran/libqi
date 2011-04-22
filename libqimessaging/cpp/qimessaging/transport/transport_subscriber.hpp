#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_TRANSPORT_TRANSPORT_SUBSCRIBER_HPP_
#define _QIMESSAGING_TRANSPORT_TRANSPORT_SUBSCRIBER_HPP_

#include <string>
#include <qimessaging/api.hpp>

namespace qi {
  namespace transport {

    namespace detail {
      class SubscriberBackend;
    };

    class TransportContext;
    class TransportSubscribeHandler;

    /// \ingroup Transport
    class QIMESSAGING_API TransportSubscriber {
    public:

      /// <summary>Constructor. </summary>
      /// <param name="ctx">[in,out] The context.</param>
      TransportSubscriber(TransportContext &ctx);

      /// <summary>Finaliser. </summary>
      virtual ~TransportSubscriber();

      /// <summary>Sets a subscribe handler. </summary>
      /// <param name="handler">The handler.</param>
      virtual void setSubscribeHandler(TransportSubscribeHandler* handler);

      /// <summary>Gets the subscribe handler. </summary>
      /// <returns>The subscribe handler.</returns>
      virtual TransportSubscribeHandler* getSubscribeHandler() const;

      /// <summary>Initialises this object. </summary>
      virtual void init();

      /// <summary>Connects. </summary>
      /// <param name="endpoint">The endpoint to connect to.</param>
      virtual void connect(const std::string &endpoint);

      virtual void subscribe();

    protected:

      /// <summary> Context for the transport </summary>
      qi::transport::TransportContext          &_transportContext;

      /// <summary> The subscriber </summary>
      qi::transport::detail::SubscriberBackend *_subscriber;

      /// <summary> The subscribe handler </summary>
      TransportSubscribeHandler                *_subscribeHandler;
    };
  }
}

#endif  // _QIMESSAGING_TRANSPORT_TRANSPORT_SUBSCRIBER_HPP_

#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_SUBSCRIBER_IMPL_HPP_
#define _QI_MESSAGING_SRC_SUBSCRIBER_IMPL_HPP_

#include <string>
#include <qimessaging/transport/transport_subscriber.hpp>
#include <qimessaging/transport/transport_subscribe_handler.hpp>
#include "src/messaging/serviceinfo.hpp"
#include "src/messaging/mutexednamelookup.hpp"
#include "src/messaging/impl_base.hpp"

namespace qi {
  namespace detail {

    class SubscriberImpl :
      public qi::detail::ImplBase,
      public qi::transport::TransportSubscribeHandler {
    public:

      explicit SubscriberImpl(const std::string& name = "", Context *ctx = 0);

      void connect(const std::string &masterAddress = "127.0.0.1:5555");

      virtual ~SubscriberImpl();

      /// <summary>Subscribes to a topic and registers a handler</summary>
      /// <param name="signature">Signature of the topic.</param>
      /// <param name="functor">The functor handler</param>
      void subscribe(const std::string& signature, qi::Functor* functor);

      /// <summary>Unsubscribes. </summary>
      /// <param name="topicName">Name of the topic.</param>
      void unsubscribe(const std::string& signature);

      /// <summary>Subscribe handler that implements the SubscribeHandler interface </summary>
      /// <param name="message"> The incoming Message.</param>
      /// <seealso cref="qi::transport::SubscribeHandler"/>
      void subscribeHandler(qi::transport::Buffer &message);

    protected:
      void xInit();
      bool xConnect(const std::string& address);
      boost::shared_ptr<qi::transport::TransportSubscriber> _transportSubscriber;
      MutexedNameLookup<ServiceInfo> _subscriberCallBacks;
      MutexedNameLookup<std::string> _subscribedEndpoints;
    };
  }
}
#endif  // _QI_MESSAGING_SRC_SUBSCRIBER_IMPL_HPP_

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
#include <qi/transport/transport_subscriber.hpp>
#include <qi/transport/transport_subscribe_handler.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/messaging/src/mutexednamelookup.hpp>
#include <qi/messaging/src/master_client.hpp>

namespace qi {
  namespace detail {

    class SubscriberImpl :
      public qi::detail::MasterClient,
      public qi::transport::TransportSubscribeHandler {
    public:

      SubscriberImpl(const std::string& name, const std::string& masterAddress = "127.0.0.1:5555");

      virtual ~SubscriberImpl();

      /// <summary>Subscribes to a topic and registers a handler</summary>
      /// <param name="topicName">Name of the topic.</param>
      /// <param name="functor">The functor handler</param>
      void subscribe(const std::string& topicName, qi::Functor* functor);

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

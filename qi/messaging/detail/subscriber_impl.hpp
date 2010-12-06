#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_DETAIL_SUBSCRIBER_IMPL_HPP_
#define _QI_MESSAGING_DETAIL_SUBSCRIBER_IMPL_HPP_

#include <string>
#include <qi/transport/subscriber.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/messaging/detail/mutexednamelookup.hpp>
#include <qi/messaging/detail/master_client.hpp>

namespace qi {
  namespace detail {

    class SubscriberImpl : public qi::detail::MasterClient, public qi::transport::SubscribeHandler {
    public:

      explicit SubscriberImpl(const std::string& name, const std::string& masterAddress = "127.0.0.1:5555");

      virtual ~SubscriberImpl();

      void subscribe(const std::string& topicName, qi::Functor* f);

      // Subscribe Handler Interface Implementation
      void subscribeHandler(qi::transport::Buffer &requestMessage);

    protected:
      void xInit();
      bool xConnect(const std::string& address);

      boost::shared_ptr<qi::transport::Subscriber> _transportSubscriber;
      MutexedNameLookup<ServiceInfo> _subscriberCallBacks;
      MutexedNameLookup<std::string> _subscribedEndpoints;
    };
  }
}
#endif  // _QI_MESSAGING_DETAIL_SUBSCRIBER_IMPL_HPP_

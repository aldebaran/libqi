#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_SUBSCRIBER_HPP_
#define _QI_TRANSPORT_TRANSPORT_SUBSCRIBER_HPP_

#include <string>

namespace qi {
  namespace transport {

    namespace detail {
      class SubscriberBackend;
    };

    class TransportSubscribeHandler;

    class TransportSubscriber {
    public:
      TransportSubscriber();
      virtual ~TransportSubscriber();

      virtual void setSubscribeHandler(TransportSubscribeHandler* callback);
      virtual TransportSubscribeHandler* getSubscribeHandler() const;

      virtual void connect(const std::string &publishAddress);
      virtual void subscribe();

    protected:
      qi::transport::detail::SubscriberBackend *_subscriber;
      TransportSubscribeHandler                *_subscribeHandler;
    };
  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_SUBSCRIBER_HPP_

#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_SUBSCRIBER_IMPL_HPP__
#define   __QI_MESSAGING_DETAIL_SUBSCRIBER_IMPL_HPP__

#include <string>
#include <boost/shared_ptr.hpp>
#include <qi/transport/subscriber.hpp>
#include <qi/messaging/detail/client_impl_base.hpp>
#include <qi/transport/subscribe_handler_user.hpp>

namespace qi {
  namespace detail {

    class SubscriberImpl : public qi::transport::SubscribeHandlerUser, public qi::detail::ClientImplBase {
    public:

      explicit SubscriberImpl(const std::string& masterAddress);

      virtual ~SubscriberImpl();

      bool connect(const std::string& address);

      void setSubscribeHandler(qi::transport::SubscribeHandler* callback);

      qi::transport::SubscribeHandler* getSubscribeHandler() const;

    protected:
      void xInit();
      boost::shared_ptr<qi::transport::Subscriber> _subscriber;
    };
  }
}
#endif // __QI_MESSAGING_DETAIL_SUBSCRIBER_IMPL_HPP__

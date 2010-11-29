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

namespace qi {
  namespace detail {

    class SubscriberImpl {
    public:
      SubscriberImpl();
      virtual ~SubscriberImpl();
    };

  }
}

#endif // __QI_MESSAGING_DETAIL_SUBSCRIBER_IMPL_HPP__

#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_PUBLISHER_IMPL_HPP__
#define   __QI_MESSAGING_DETAIL_PUBLISHER_IMPL_HPP__

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <qi/messaging/detail/client_impl_base.hpp>

namespace qi {
  namespace transport {
    class Publisher;
  }

  namespace detail {

    class PublisherImpl : ClientImplBase{
    public:
      PublisherImpl(const std::string& masterAddress);
      virtual ~PublisherImpl();

      bool bind(const std::vector<std::string> &publishAddresses);

      void publish(const std::string& data);

    protected:
      void xInit();
      const std::string& _masterAddress;
      boost::scoped_ptr<qi::transport::Publisher> _publisher;
    };

  }
}

#endif // __QI_MESSAGING_DETAIL_PUBLISHER_IMPL_HPP__

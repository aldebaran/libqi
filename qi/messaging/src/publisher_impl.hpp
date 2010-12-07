#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_DETAIL_PUBLISHER_IMPL_HPP_
#define _QI_MESSAGING_DETAIL_PUBLISHER_IMPL_HPP_

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <qi/messaging/detail/master_client.hpp>

namespace qi {
  namespace transport {
    class TransportPublisher;
  }

  namespace detail {

    class PublisherImpl : MasterClient {
    public:
      PublisherImpl(const std::string& name, const std::string& masterAddress);
      virtual ~PublisherImpl();

      void advertiseTopic(const std::string& signature);
      void publish(const std::string& data);

    protected:
      void xInitPublisher();
      bool xBind(const std::vector<std::string> &publishAddresses);

      bool _publisherInitialized;
      boost::scoped_ptr<qi::transport::TransportPublisher> _publisher;
    };

  }
}

#endif  // _QI_MESSAGING_DETAIL_PUBLISHER_IMPL_HPP_

#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_PUBLISHER_IMPL_HPP_
#define _QI_MESSAGING_SRC_PUBLISHER_IMPL_HPP_

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <qi/messaging/src/impl_base.hpp>

namespace qi {
  namespace transport {
    class TransportPublisher;
  }

  namespace detail {

    class PublisherImpl : ImplBase {
    public:
      PublisherImpl(const std::string& name = "", Context *ctx = 0);
      virtual ~PublisherImpl();
      void connect(const std::string &masterAddress = "127.0.0.1:5555");

      /// <summary>Advertises a topic. </summary>
      /// <param name="signature">The signature of the topic</param>
      /// <param name="isManyToMany">Allows many to many publishing</param>
      void advertiseTopic(const std::string& signature,
        const bool& isManyToMany);

      /// <summary>Unadvertise a topic. </summary>
      /// <param name="signature">The signature of the topic.</param>
      void unadvertiseTopic(const std::string& signature);

      /// <summary>Publishes a serialized message</summary>
      /// <param name="message">The serialized message.</param>
      void publish(const std::string& message);

    protected:
      void xInitPublisher();
      bool xBind(const std::vector<std::string> &publishAddresses);
      bool _publisherInitialized;
      boost::scoped_ptr<qi::transport::TransportPublisher> _publisher;
    };

  }
}

#endif  // _QI_MESSAGING_SRC_PUBLISHER_IMPL_HPP_

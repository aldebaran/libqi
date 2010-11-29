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

namespace qi {
  namespace transport {
    class Publisher;
  }

  namespace detail {

    class PublisherImpl {
    public:
      PublisherImpl();
      virtual ~PublisherImpl();

      bool bind(const std::string &address);

      void publish(const std::string& data);

      bool isInitialized() const;

    protected:
      bool _isInitialized;
      qi::transport::Publisher* _publisher;
    };

  }
}

#endif // __QI_MESSAGING_DETAIL_PUBLISHER_IMPL_HPP__

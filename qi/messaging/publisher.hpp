/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_MESSAGING_PUBLISHER_HPP__
#define   __QI_MESSAGING_PUBLISHER_HPP__

#include <string>
#include <boost/shared_ptr.hpp>
#include <qi/serialization.hpp>
#include <qi/messaging/detail/publisher_impl.hpp> // TODO use an interface

namespace qi {
  namespace detail {
    class PublisherImpl;
  }

    template<typename T>
    class Publisher {
    public:
      Publisher(
        boost::shared_ptr<qi::detail::PublisherImpl> impl) : _impl(impl) {}

      void publish(const T& val)
      {
        qi::serialization::BinarySerializer ser;
        qi::serialization::serialize<T>::write(ser, val);
        _impl->publish(ser.str());
      }

    protected:
      boost::shared_ptr<qi::detail::PublisherImpl> _impl;
    };
}

#endif // __QI_MESSAGING_PUBLISHER_HPP__

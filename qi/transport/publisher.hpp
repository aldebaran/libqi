/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_TRANSPORT_PUBLISHER_HPP_
# define QI_TRANSPORT_PUBLISHER_HPP_

#include <string>

namespace qi {
  namespace transport {

    class Publisher {
    public:
      explicit Publisher(const std::string& publishAddress)
        : _publishAddress(publishAddress) {}
      virtual ~Publisher() {}

      virtual void publish(const std::string& tosend) = 0;
    protected:
      std::string _publishAddress;
    };
  }
}

#endif  // QI_TRANSPORT_PUBLISHER_HPP_

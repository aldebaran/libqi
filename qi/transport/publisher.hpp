/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_PUBLISHER_HPP__
#define   __QI_TRANSPORT_PUBLISHER_HPP__

#include <string>
#include <vector>

namespace qi {
  namespace transport {

    class Publisher {
    public:
      Publisher() {}
      virtual ~Publisher() {}

      virtual void connect(const std::string& serverAddresses) = 0;
      virtual void bind(const std::string& serverAddress) = 0;
      virtual void bind(const std::vector<std::string>& serverAddresses) = 0;
      virtual void publish(const std::string& tosend) = 0;
    };
  }
}

#endif // __QI_TRANSPORT_PUBLISHER_HPP__

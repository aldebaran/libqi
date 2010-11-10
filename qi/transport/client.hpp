/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_TRANSPORT_CLIENT_HPP_
# define AL_TRANSPORT_CLIENT_HPP_

#include <string>

namespace qi {
  namespace Transport {

    class Client {
    public:
      explicit Client(const std::string &serverAddress)
        : _serverAddress(serverAddress) {}
      virtual ~Client() {}

      virtual void send(const std::string &tosend, std::string &result) = 0;
    protected:
      std::string _serverAddress;
    };
  }
}

#endif  // AL_TRANSPORT_CLIENT_HPP_

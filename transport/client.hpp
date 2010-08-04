/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	_AL_MESSAGING_TRANSPORT_CLIENTBASE_HPP_
# define   	_AL_MESSAGING_TRANSPORT_CLIENTBASE_HPP_

# include <string>

namespace AL {
  namespace Transport {

  class Client {
  public:
    Client(const std::string &serverAddress)
      : _serverAddress(serverAddress)
    {}

    virtual void send(const std::string &tosend, std::string &result) = 0;

  protected:
    std::string         _serverAddress;
  };
}
}

#endif	    /* !_AL_MESSAGING_TRANSPORT_CLIENTBASE_HPP_ */

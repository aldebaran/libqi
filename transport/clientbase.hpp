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
  namespace Messaging {

  class ClientBase {
  public:
    ClientBase(const std::string &servername)
      : server_name(servername)
    {}

    virtual void send(const std::string &tosend, std::string &result);

  protected:
    std::string         server_name;
  };
}
}

#endif	    /* !_AL_MESSAGING_TRANSPORT_CLIENTBASE_HPP_ */

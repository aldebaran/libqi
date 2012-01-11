/*
** transport-server.hpp
** Login : <hcuche@hcuche-de>
** Started on  Wed Jan 11 10:19:42 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#ifndef   	TRANSPORT_SERVER_HPP_
# define   	TRANSPORT_SERVER_HPP_

# include <string>

# include <qi/macro.hpp>
# include <event2/event.h>


class TransportServer
{
  QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

public:
  explicit TransportServer(const std::string &adress,
                           unsigned short port);
  virtual ~TransportServer();

  void run(struct event_base *base);

protected:
private:
  std::string    _adress;
  unsigned short _port;
};


#endif	    /* !TRANSPORT_SERVER_HPP_ */

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

class TransportServerDelegate
{
public:
  virtual void onConnected(const std::string &msg = "") = 0;
  virtual void onWrite(const std::string &msg = "")     = 0;
  virtual void onRead(const std::string &msg = "")      = 0;
};


class TransportServer
{
  QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

public:
  explicit TransportServer(const std::string &address,
                           unsigned short port);
  virtual ~TransportServer();

  void run(struct event_base *base);

  void setDelegate(TransportServerDelegate *delegate);
  TransportServerDelegate *_tsd;

  struct event_base *_base;
private:
  std::string    _address;
  unsigned short _port;

};


#endif	    /* !TRANSPORT_SERVER_HPP_ */

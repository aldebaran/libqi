/*
** network-thread.hpp
** Login : <hcuche@hcuche-de>
** Started on  Tue Jan 10 11:41:39 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#pragma once
#ifndef _SRC_NETWORK_THREAD_HPP_
#define _SRC_NETWORK_THREAD_HPP_

# include <event2/event.h>
# include <event2/bufferevent.h>

# include <boost/thread.hpp>

namespace qi {

class NetworkThread
{
public:
  NetworkThread();
  ~NetworkThread();

  void join();
  void stop();
  struct event_base* getEventBase();

protected:
private:
  void run();

  struct event_base *_base;
  boost::thread      _thd;
};
}

#endif  // _SRC_NETWORK_THREAD_HPP_

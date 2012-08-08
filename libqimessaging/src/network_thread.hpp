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
# include <boost/shared_ptr.hpp>

namespace qi {

class NetworkThread
{
public:
  NetworkThread();


  void join();
  void stop();
  void destroy(bool join);
  struct event_base* getEventBase();
  struct AsyncCallHandler
  {
    void cancel() { cancelled = true;}
    bool cancelled;
    event* ev;
    boost::function0<void> callback;
  };
  /// Call given function once after given delay in microseconds.
  boost::shared_ptr<AsyncCallHandler> asyncCall(uint64_t usDelay, boost::function0<void> callback);

protected:
private:
  ~NetworkThread();
  void run();
  struct event_base *_base;
  boost::thread      _thd;
  bool               _destroyMe;
  bool               _running;
  boost::recursive_mutex _mutex;
};
}

#endif  // _SRC_NETWORK_THREAD_HPP_

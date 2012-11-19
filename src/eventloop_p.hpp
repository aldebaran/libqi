#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_EVENTLOOP_P_HPP_
#define _SRC_EVENTLOOP_P_HPP_

#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <qi/eventloop.hpp>

namespace qi {
  class AsyncCallHandlePrivate
  {
  public:
    void cancel() { cancelled = true;}
    bool cancelled;
    event* ev;
    // Feature supported only in the case of fd monitoring.
    // If true, callback will be called again and again.
    // If false it will be called only once.
    bool persistant;
    // Callback used with asyncCall.
    boost::function<void()> callback;
    // Callback used with notifyFd.
    EventLoop::NotifyFdCallbackFunction fdcallback;
  };

  class EventLoopPrivate
  {
  public:
    EventLoopPrivate();
    bool isInEventLoopThread();
    void start();
    void join();
    void stop();
    EventLoop::AsyncCallHandle asyncCall(uint64_t usDelay,
      boost::function<void ()> callback);
    EventLoop::AsyncCallHandle notifyFd(evutil_socket_t fd,
      EventLoop::NotifyFdCallbackFunction cb, short evflags, bool persistant);
    void destroy(bool join);
    event_base* getEventBase() { return _base;}
    void run();
  private:
    ~EventLoopPrivate();
    struct event_base* _base;
    boost::thread      _thd;
    bool               _destroyMe;
    bool               _running;
    bool               _threaded;
    boost::recursive_mutex _mutex;
    boost::thread::id  _id;
  };
}

#endif  // _SRC_EVENTLOOP_P_HPP_

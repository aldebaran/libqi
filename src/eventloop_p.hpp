#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_EVENTLOOP_P_HPP_
#define _SRC_EVENTLOOP_P_HPP_

#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <boost/asio.hpp>
#include <qi/eventloop.hpp>

namespace qi {
  class AsyncCallHandlePrivate
  {
  public:
    void cancel() { cancelled = true;}
    bool cancelled;
    // Callback used with notifyFd.
#ifdef _WIN32
    // Assume it is a socket under WIN32, no way to poll
    // a generic descriptor.
    typedef boost::asio::ip::tcp::socket Stream;
#else
    typedef boost::asio::posix::stream_descriptor Stream;
#endif
    EventLoop::NotifyFdCallbackFunction fdcallback;
    Stream* sd;
  };

  class EventLoopPrivate
  {
  public:
    virtual bool isInEventLoopThread()=0;
    virtual void start()=0;
    virtual void join()=0;
    virtual void stop()=0;
    virtual qi::Future<void>   asyncCall(uint64_t usDelay,
      boost::function<void ()> callback)=0;
    virtual void post(uint64_t usDelay,
      const boost::function<void ()>& callback)=0;
    virtual EventLoop::AsyncCallHandle notifyFd(int fd,
      EventLoop::NotifyFdCallbackFunction cb, EventLoop::FileOperation fdUsage)=0;
    virtual void destroy(bool join)=0;
    virtual void* nativeHandle()=0;
    virtual void run()=0;
  protected:
    virtual ~EventLoopPrivate() {}
  };

  class EventLoopAsio: public EventLoopPrivate
  {
  public:
    EventLoopAsio();
    virtual bool isInEventLoopThread();
    virtual void start();
    virtual void run();
    virtual void join();
    virtual void stop();
    virtual qi::Future<void>   asyncCall(uint64_t usDelay,
      boost::function<void ()> callback);
    virtual void post(uint64_t usDelay,
      const boost::function<void ()>& callback);
    EventLoop::AsyncCallHandle notifyFd(int fd,
      EventLoop::NotifyFdCallbackFunction cb, EventLoop::FileOperation fdUsage);
    virtual void destroy(bool join);
    virtual void* nativeHandle();
  private:
    virtual ~EventLoopAsio();
    boost::asio::io_service _io;
    boost::thread      _thd;
    bool               _destroyMe;
    bool               _running;
    bool               _threaded;
    boost::recursive_mutex _mutex;
    boost::thread::id  _id;
  };

  class ThreadPool;
  class EventLoopThreadPool: public EventLoopPrivate
  {
  public:
    EventLoopThreadPool(int minWorkers, int maxWorkers, int minIdleWorkers, int maxIdleWorkers);
    virtual bool isInEventLoopThread();
    virtual void start();
    virtual void run();
    virtual void join();
    virtual void stop();
    virtual qi::Future<void>   asyncCall(uint64_t usDelay,
      boost::function<void ()> callback);
    virtual void post(uint64_t usDelay,
      const boost::function<void ()>& callback);
    EventLoop::AsyncCallHandle notifyFd(int fd,
      EventLoop::NotifyFdCallbackFunction cb, EventLoop::FileOperation fdUsage);
    virtual void destroy(bool join);
    virtual void* nativeHandle();
  private:
    virtual ~EventLoopThreadPool();
    ThreadPool* _pool;
    bool _stopping;
  };
}

#endif  // _SRC_EVENTLOOP_P_HPP_

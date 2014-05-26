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
    AsyncCallHandlePrivate() { }
    void cancel() { canceled = true;}
    bool canceled;
    // Callback used with notifyFd.
#ifdef _WIN32
    // Assume it is a socket under WIN32, no way to poll
    // a generic descriptor.
    typedef boost::asio::ip::tcp::socket Stream;
#else
    typedef boost::asio::posix::stream_descriptor Stream;
#endif
    Stream* sd;
  };

  class EventLoopPrivate
  {
  public:
    virtual bool isInEventLoopThread()=0;
    virtual void start(int nthreads)=0; // 0=auto
    virtual void join()=0;
    virtual void stop()=0;
    virtual qi::Future<void> asyncCall(uint64_t usDelay, boost::function<void ()> callback)=0;
    virtual void post(uint64_t usDelay, const boost::function<void ()>& callback)=0;
    virtual void destroy()=0;
    virtual void* nativeHandle()=0;
    virtual void run()=0;
    virtual void setMaxThreads(unsigned int max)=0;
    boost::function<void()> _emergencyCallback;
  protected:
    virtual ~EventLoopPrivate() {}
  };

  class EventLoopAsio: public EventLoopPrivate
  {
  public:
    EventLoopAsio();
    virtual bool isInEventLoopThread();
    virtual void start(int nthreads);
    virtual void run();
    virtual void join();
    virtual void stop();
    virtual qi::Future<void>   asyncCall(uint64_t usDelay,
      boost::function<void ()> callback);
    virtual void post(uint64_t usDelay,
      const boost::function<void ()>& callback);
    virtual void destroy();
    virtual void* nativeHandle();
    virtual void setMaxThreads(unsigned int max);
  private:
    void invoke_maybe(boost::function<void()> f, qi::uint32_t id, qi::Promise<void> p, const boost::system::error_code& erc);
    void _runPool();
    void _pingThread();
    virtual ~EventLoopAsio();

    enum Mode
    {
      Mode_Unset = 0,
      Mode_Threaded = 1,
      Mode_Pooled = 2
    };
    Mode _mode;
    qi::Atomic<int> _nThreads;
    boost::asio::io_service _io;
    boost::asio::io_service::work* _work; // keep io.run() alive
    boost::thread      _thd;
    bool               _destroyMe;
    qi::Atomic<int>    _running;
    boost::recursive_mutex _mutex;
    boost::thread::id  _id;
    unsigned int _maxThreads;

    qi::Atomic<uint32_t> _totalTask;
    qi::Atomic<uint32_t> _activeTask;
  };

  class ThreadPool;
  class EventLoopThreadPool: public EventLoopPrivate
  {
  public:
    EventLoopThreadPool(int minWorkers, int maxWorkers, int minIdleWorkers, int maxIdleWorkers);
    virtual bool isInEventLoopThread();
    virtual void start(int nthreads);
    virtual void run();
    virtual void join();
    virtual void stop();
    virtual qi::Future<void>   asyncCall(uint64_t usDelay,
      boost::function<void ()> callback);
    virtual void post(uint64_t usDelay,
      const boost::function<void ()>& callback);
    virtual void destroy();
    virtual void* nativeHandle();
    virtual void setMaxThreads(unsigned int max);
  private:
    virtual ~EventLoopThreadPool();
    void _destroy();
    ThreadPool* _pool;
    bool _stopping;
  };
}

#endif  // _SRC_EVENTLOOP_P_HPP_

#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_EVENTLOOP_P_HPP_
#define _SRC_EVENTLOOP_P_HPP_

#include <atomic>
#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <boost/asio.hpp>
#include <qi/eventloop.hpp>

namespace qi {
  class AsyncCallHandlePrivate
  {
  public:
    AsyncCallHandlePrivate()
      : canceled(false)
      , sd(NULL)
    { }

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
    virtual bool isInThisContext()=0;
    virtual void start(int nthreads)=0; // 0=auto
    virtual void join()=0;
    virtual void stop()=0;
    virtual qi::Future<void> asyncCall(qi::Duration delay, boost::function<void ()> callback)=0;
    virtual void post(qi::Duration delay, const boost::function<void ()>& callback)=0;
    virtual qi::Future<void> asyncCall(qi::SteadyClockTimePoint timepoint, boost::function<void ()> callback)=0;
    virtual void post(qi::SteadyClockTimePoint timepoint, const boost::function<void ()>& callback)=0;
    virtual void destroy()=0;
    virtual void* nativeHandle()=0;
    virtual void setMaxThreads(unsigned int max)=0;
    boost::function<void()> _emergencyCallback;
    std::string             _name;

  protected:
    virtual ~EventLoopPrivate() = default;
  };

  class EventLoopAsio final: public EventLoopPrivate
  {
  public:
    EventLoopAsio();
    bool isInThisContext() override;
    void start(int nthreads) override;
    void join() override;
    void stop() override;
    qi::Future<void> asyncCall(qi::Duration delay,
      boost::function<void ()> callback) override;
    void post(qi::Duration delay,
      const boost::function<void ()>& callback) override;
    qi::Future<void> asyncCall(qi::SteadyClockTimePoint timepoint,
        boost::function<void ()> callback) override;
    void post(qi::SteadyClockTimePoint timepoint,
        const boost::function<void ()>& callback) override;
    void destroy() override;
    void* nativeHandle() override;
    void setMaxThreads(unsigned int max) override;
  private:
    void invoke_maybe(boost::function<void()> f, qi::uint32_t id, qi::Promise<void> p, const boost::system::error_code& erc);
    void _runPool();
    void _pingThread();
    ~EventLoopAsio() override;

    enum class Mode
    {
      Unset = 0,
      Threaded = 1,
      Pooled = 2
    };
    Mode _mode;
    qi::Atomic<unsigned int> _nThreads;
    boost::asio::io_service _io;
    std::atomic<boost::asio::io_service::work*> _work; // keep io.run() alive
    boost::thread      _thd;
    qi::Atomic<int>    _running;
    boost::recursive_mutex _mutex;
    boost::thread::id  _id;
    unsigned int _maxThreads;

    class WorkerThreadPool;
    boost::scoped_ptr<WorkerThreadPool> _workerThreads;

    qi::Atomic<uint32_t> _totalTask;
    qi::Atomic<uint32_t> _activeTask;
  };
}

#endif  // _SRC_EVENTLOOP_P_HPP_

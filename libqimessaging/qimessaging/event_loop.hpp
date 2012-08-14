/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_EXECUTIONCONTEXT_HPP_
#define _QIMESSAGING_EXECUTIONCONTEXT_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <qi/types.hpp>
#include <qimessaging/api.hpp>

namespace qi
{
  class EventLoopPrivate;
  class AsyncCallHandlePrivate;
  class QIMESSAGING_API EventLoop
  {
  public:
    /** Create a new eventLoop.
     * You must then call eiter start() or run() to start event processing.
    */
    EventLoop();
    ~EventLoop();
    /// Return true if current thread is the event loop thread.
    bool isInEventLoopThread();
    /// Start in a thread (in case useThread=false was passed to constructor).
    void start();
    /// Wait for run thread to terminate
    void join();
    /// Ask main loop to terminate
    void stop();
    /// Run main loop in current thread.
    void run();

    class AsyncCallHandle
    {
    public:
      AsyncCallHandle();
      ~AsyncCallHandle();
      /// Cancel the call if it was not already processed
      void cancel();

      boost::shared_ptr<AsyncCallHandlePrivate> _p;
    };
    /// Call given function once after given delay in microseconds.
    AsyncCallHandle asyncCall(uint64_t usDelay,
      boost::function<void ()> callback);

    EventLoopPrivate *_p;
  };

  /// Return a default event loop for network operations.
  QIMESSAGING_API EventLoop* getDefaultNetworkEventLoop();
  /// Return a default context for other uses.
  QIMESSAGING_API EventLoop* getDefaultObjectEventLoop();
}

#endif

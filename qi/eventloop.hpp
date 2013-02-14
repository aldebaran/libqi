#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_EVENTLOOP_HPP_
#define _QIMESSAGING_EVENTLOOP_HPP_

#ifdef _MSC_VER
#  pragma warning( disable: 4503 ) // decorated name length
#endif

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <qi/types.hpp>
#include <qi/api.hpp>
#include <qi/future.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi
{
  class EventLoopPrivate;
  class AsyncCallHandlePrivate;
  class QI_API EventLoop
  {
  public:
    /** Create a new eventLoop.
     * You must then call eiter start(), run() or startThreadPool() to start event processing.
    */
    EventLoop();
    ~EventLoop();
    /// Return true if current thread is the event loop thread.
    bool isInEventLoopThread();
    /// Start in a thread (in case useThread=false was passed to constructor).
    void start();
    /// Start in thread-pool mode: each asyncCall() will be run in parallel
    void startThreadPool(int minWorkers=-1, int maxWorkers=-1, int minIdleWorkers=-1, int maxIdleWorkers=-1);
    /// Wait for run thread to terminate
    void join();
    /// Ask main loop to terminate
    void stop();
    /// Run main loop in current thread.
    void run();

    // Internal function
    void *nativeHandle();

    class QI_API AsyncCallHandle
    {
    public:
      AsyncCallHandle();
      ~AsyncCallHandle();
      /// Cancel the call if it was not already processed
      void cancel();

    public:
      // CS4251
      boost::shared_ptr<AsyncCallHandlePrivate> _p;
    };

    /** Call given function once after given delay in microseconds.
     * @return a cancelleable future
     */
    Future<void> async(boost::function<void ()> callback, uint64_t usDelay=0);
    /// Similar to asyncCall() but without cancellation or notification
    void post(const boost::function<void ()>& callback, uint64_t usDelay=0);

    /** Monitor event loop to detect deadlocks.
     @param helper an other event loop used for monitoring
     @param maxUsDelay maximum expected delay between an async() and its execution
     @return a cancelleable future. Invoke cancel() to terminate monitoring.
            In case an async() call does not execute in time, the
            future's error will be set.
    */
    Future<void> monitorEventLoop(EventLoop* helper, uint64_t maxUsDelay);


    EventLoopPrivate *_p;
  };

  /// Return a default event loop for network operations.
  QI_API EventLoop* getDefaultNetworkEventLoop();
  /// Return a default context for other uses.
  QI_API EventLoop* getDefaultObjectEventLoop();
  /// Return a default thread pool context
  QI_API EventLoop* getDefaultThreadPoolEventLoop();
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QIMESSAGING_EVENTLOOP_HPP_

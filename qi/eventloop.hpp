#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_EVENTLOOP_HPP_
#define _QIMESSAGING_EVENTLOOP_HPP_

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

    enum QI_API FileOperation
    {
      FileOperation_Read = 1,
      FileOperation_Write = 2,
      FileOperation_ReadOrWrite = FileOperation_Read | FileOperation_Write
    };

    typedef boost::function<void(int, FileOperation)> NotifyFdCallbackFunction;

    /** Call given function once after given delay in microseconds.
     * @return a cancelleable future
     */
    Future<void> asyncCall(uint64_t usDelay, boost::function<void ()> callback);
    /// Call given function every time something happen on file
    /// descriptor fileDescriptor. You can specify that the callback
    /// will be called for every Read, every Write or for both Read
    /// and Write operations, using condition variable.
    AsyncCallHandle notifyFd(int fileDescriptor, NotifyFdCallbackFunction callback, FileOperation condition);

    EventLoopPrivate *_p;
  };

  /// Return a default event loop for network operations.
  QI_API EventLoop* getDefaultNetworkEventLoop();
  /// Return a default context for other uses.
  QI_API EventLoop* getDefaultObjectEventLoop();
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QIMESSAGING_EVENTLOOP_HPP_

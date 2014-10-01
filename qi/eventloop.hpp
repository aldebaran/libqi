#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_EVENTLOOP_HPP_
# define _QI_EVENTLOOP_HPP_

# ifdef _MSC_VER
#  pragma warning( disable: 4503 ) // decorated name length
# endif

# include <boost/shared_ptr.hpp>
# include <boost/function.hpp>

# include <qi/types.hpp>
# include <qi/api.hpp>
# include <qi/clock.hpp>
# include <qi/detail/executioncontext.hpp>

# ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
# endif

namespace boost {
  namespace asio {
    class io_service;
}}

namespace qi
{
  template<typename T> class Future;

  class EventLoopPrivate;
  /**
   * \brief Class to handle eventloop.
   * \includename{qi/eventloop.hpp}
   */
  class QI_API EventLoop : public ExecutionContext
  {
  public:
    /**
     * \brief Create a new eventLoop.
     * \param name Name of the event loop created.
     *
     * You must then call either start(), run() or startThreadPool() to start event processing.
     */
    EventLoop(const std::string& name = "eventloop");

    /// \brief Default destructor.
    ~EventLoop();
    /**
     * \brief Check if current thread is the event loop thread.
     * \return true if current thread is the event loop thread.
     */
    bool isInThisContext();
    /**
     * \brief Start the eventloop in threaded mode.
     * \param nthreads Numbers of threads.
     */
    void start(int nthreads = 0);

    /// \brief Wait for run thread to terminate.
    void join();
    /// \brief Ask main loop to terminate.
    void stop();
    /**
     * \brief Set callback to be called in case of a deadlock detection.
     * \param cb Callback to be called.
     */
    void setEmergencyCallback(boost::function<void()> cb);

    /**
     * \brief Set the maximum number of threads in the pool.
     * \param max Maximum number of threads.
     */
    void setMaxThreads(unsigned int max);

    /// \brief Internal function.
    void *nativeHandle();

    /// @{
    /**
     * \brief Call given function once after given delay in microseconds.
     * \param callback Callback to be called.
     * \param usDelay Delay before call the callback in microsecond.
     * \return A canceleable future.
     * \deprecated use qi::async with qi::Duration
     */
    template<typename R>
    Future<R> async(const boost::function<R()>& callback, uint64_t usDelay=0);
    Future<void> async(const boost::function<void ()>& callback, uint64_t usDelay=0);
    Future<void> async(const boost::function<void ()>& callback, qi::Duration delay);
    Future<void> async(const boost::function<void ()>& callback, qi::SteadyClockTimePoint timepoint);

    template <typename R>
    typename boost::enable_if_c<!boost::is_same<R, void>::value,
                                qi::Future<R> >::type
        async(const boost::function<R()>& callback, qi::Duration delay)
    {
      return ExecutionContext::async<R>(callback, delay);
    }
    template <typename R>
    typename boost::enable_if_c<!boost::is_same<R, void>::value,
                                qi::Future<R> >::type
        async(const boost::function<R()>& callback,
              qi::SteadyClockTimePoint tp)
    {
      return ExecutionContext::async<R>(callback, tp);
    }
    /// @}

    /**
     * \brief Similar to async() but without cancelation or notification.
     * \param callback Callback to be called.
     * \param usDelay Delay before call the callback in microsecond.
     */
    void post(const boost::function<void ()>& callback, uint64_t usDelay);
    void post(const boost::function<void ()>& callback, qi::Duration delay);
    void post(const boost::function<void ()>& callback, qi::SteadyClockTimePoint timepoint);
    void post(const boost::function<void ()>& callback)
    {
      return post(callback, 0);
    }

    /**
     * \brief Monitor event loop to detect deadlocks.
     * \param helper an other event loop used for monitoring.
     * \param maxUsDelay maximum expected delay between an async() and its execution.
     * \return A canceleable future. Invoke cancel() to terminate monitoring.
     *         In case an async() call does not execute in time, the
     *         future's error will be set.
     */
    Future<void> monitorEventLoop(EventLoop* helper, uint64_t maxUsDelay);

    EventLoopPrivate *_p;
    std::string       _name;
  };

  /// \brief Return the global eventloop, created on demand on first call.
  QI_API EventLoop* getEventLoop();

  namespace detail
  {
    template <typename R, typename ARG0>
    typename boost::enable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        Future<R> >::type
        asyncMaybeActor(const ARG0& arg0, const boost::function<R()>& cb,
                        qi::Duration delay);
    template <typename R, typename ARG0>
    typename boost::disable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        Future<R> >::type
        asyncMaybeActor(const ARG0& arg0, const boost::function<R()>& cb,
                        qi::Duration delay);
    template <typename R, typename ARG0>
    typename boost::enable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        Future<R> >::type
        asyncMaybeActor(const ARG0& arg0, const boost::function<R()>& cb,
                        qi::SteadyClockTimePoint timepoint);
    template <typename R, typename ARG0>
    typename boost::disable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        Future<R> >::type
        asyncMaybeActor(const ARG0& arg0, const boost::function<R()>& cb,
                        qi::SteadyClockTimePoint timepoint);
  }

  /// \copydoc qi::EventLoop::async().
  /// \deprecated use qi::async with qi::Duration
  template<typename R>
  inline Future<R> async(boost::function<R()> callback, uint64_t usDelay=0)
  {
    return qi::getEventLoop()->async(callback, usDelay);
  }
  template<typename R>
  inline Future<R> async(boost::function<R()> callback, qi::Duration delay)
  {
    return qi::getEventLoop()->async(callback, delay);
  }
  template<typename R>
  inline Future<R> async(boost::function<R()> callback, qi::SteadyClockTimePoint timepoint)
  {
    return qi::getEventLoop()->async(callback, timepoint);
  }

#ifdef DOXYGEN
  template<typename R, typename Func, typename ArgTrack>
  qi::Future<R> async(const Func& f, const ArgTrack& toTrack, ...);
#else
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                  \
  template<typename R, typename AF, typename ARG0 comma ATYPEDECL>         \
  inline typename boost::disable_if<                                       \
      boost::mpl::or_<                                                     \
          boost::is_same<ARG0, std::string>, boost::is_same<ARG0, char*>,  \
          boost::is_same<ARG0, const char*>, boost::is_array<ARG0> >,      \
      Future<R> >::type async(const AF& fun, const ARG0& arg0 comma ADECL, \
                              qi::Duration delay = qi::Duration(0))        \
  {                                                                        \
    return detail::asyncMaybeActor<R, ARG0>(                               \
        arg0, qi::bind<R()>(fun, arg0 comma AUSE), delay);                 \
  }                                                                        \
  template<typename R, typename AF, typename ARG0 comma ATYPEDECL>         \
  inline Future<R> async(const AF& fun, const ARG0& arg0 comma ADECL,      \
                         qi::SteadyClockTimePoint timepoint)               \
  {                                                                        \
    return detail::asyncMaybeActor<R, ARG0>(                               \
        arg0, qi::bind<R()>(fun, arg0 comma AUSE), timepoint);             \
  }
  QI_GEN(genCall)
#undef genCall
#endif

  /**
   * \brief Start the eventloop with nthread threads. No-op if already started.
   * \param nthread Set the minimum number of worker threads in the pool.
   */
  QI_API void startEventLoop(int nthread);

  /**
   * \brief Get the io_service used by the global event loop.
   * \return io_service used by the global event loop.
   */
  QI_API boost::asio::io_service& getIoService();

  namespace detail {
    /* when throw this thread will stop a thread of the eventloop
     */
    class TerminateThread {
    };
  };
}

# ifdef _MSC_VER
#  pragma warning( pop )
# endif

# include <qi/detail/eventloop.hxx>
#endif  // _QI_EVENTLOOP_HPP_

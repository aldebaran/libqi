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
#  pragma warning( disable: 4996 ) // TODO: Reactivate this warning once msvc stop triggerring a warning on overloading a deprecated function
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
    bool isInThisContext() override;
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

    // DEPRECATED
    /// @{
    /**
     * \brief Call given function once after given delay in microseconds.
     * \param callback Callback to be called.
     * \param usDelay Delay before call the callback in microsecond.
     * \return A canceleable future.
     * \deprecated use qi::async with qi::Duration
     */
    template<typename R>
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    Future<R> async(const boost::function<R()>& callback, uint64_t usDelay);
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    Future<void> async(const boost::function<void()>& callback, uint64_t usDelay)
    {
      return asyncDelayImpl(callback, qi::MicroSeconds(usDelay));
    }
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    Future<void> async(const boost::function<void()>& callback, qi::Duration delay) override
    {
      return asyncDelayImpl(callback, delay);
    }
    QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
    Future<void> async(
        const boost::function<void()>& callback, qi::SteadyClockTimePoint timepoint) override
    {
      return asyncAt(callback, timepoint);
    }

    using ExecutionContext::async;
    /// @}

    /**
     * \brief Similar to async() but without cancelation or notification.
     * \param callback Callback to be called.
     * \param usDelay Delay before call the callback in microsecond.
     */
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    void post(const boost::function<void ()>& callback, uint64_t usDelay)
    {
      postDelayImpl(callback, qi::MicroSeconds(usDelay));
    }
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    void post(const boost::function<void ()>& callback, qi::Duration delay)
    {
      postDelayImpl(callback, delay);
    }
    QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
    void post(const boost::function<void ()>& callback, qi::SteadyClockTimePoint timepoint);
    // END DEPRECATED

    using ExecutionContext::post;

    /**
     * \brief Monitor event loop to detect deadlocks.
     * \param helper an other event loop used for monitoring.
     * \param maxUsDelay maximum expected delay between an async() and its execution.
     * \return A canceleable future. Invoke cancel() to terminate monitoring.
     *         In case an async() call does not execute in time, the
     *         future's error will be set.
     */
    Future<void> monitorEventLoop(EventLoop* helper, uint64_t maxUsDelay);

  private:
    EventLoopPrivate *_p;
    std::string       _name;

    void postImpl(boost::function<void()> callback) override
    {
      postDelayImpl(callback, qi::Duration(0));
    }
    void postDelayImpl(boost::function<void()> callback, qi::Duration delay);
    qi::Future<void> asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp) override;
    qi::Future<void> asyncDelayImpl(boost::function<void()> cb, qi::Duration delay) override;
  };

  /// \brief Return the global eventloop, created on demand on first call.
  QI_API EventLoop* getEventLoop();

  namespace detail
  {
    template <typename F>
    inline auto asyncMaybeActor(F&& cb, qi::Duration delay) ->
        typename std::enable_if<detail::IsAsyncBind<F>::value, typename std::decay<decltype(cb())>::type>::type;
    template <typename F>
    inline auto asyncMaybeActor(F&& cb, qi::Duration delay) ->
        typename std::enable_if<!detail::IsAsyncBind<F>::value,
                 qi::Future<typename std::decay<decltype(cb())>::type>>::type;
    template <typename F>
    inline auto asyncMaybeActor(F&& cb, qi::SteadyClockTimePoint timepoint) ->
        typename std::enable_if<detail::IsAsyncBind<F>::value, typename std::decay<decltype(cb())>::type>::type;
    template <typename F>
    inline auto asyncMaybeActor(F&& cb, qi::SteadyClockTimePoint timepoint) ->
        typename std::enable_if<!detail::IsAsyncBind<F>::value,
                 qi::Future<typename std::decay<decltype(cb())>::type>>::type;
  }

  /// \copydoc qi::EventLoop::async().
  /// \deprecated use qi::async with qi::Duration
  template<typename R>
  QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
  inline Future<R> async(boost::function<R()> callback, uint64_t usDelay)
  {
    return qi::getEventLoop()->asyncDelay(callback, qi::MicroSeconds(usDelay));
  }
  template<typename R>
  QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
  inline Future<R> async(boost::function<R()> callback, qi::Duration delay)
  {
    return qi::getEventLoop()->asyncDelay(callback, delay);
  }
  template<typename R>
  QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
  inline Future<R> async(boost::function<R()> callback, qi::SteadyClockTimePoint timepoint)
  {
    return qi::getEventLoop()->asyncAt(callback, timepoint);
  }
  template<typename R>
  QI_API_DEPRECATED_MSG(Use 'async' without explicit return type template arguement instead)
  inline Future<R> async(detail::Function<R()> callback)
  {
    return qi::getEventLoop()->async(callback);
  }

  template <typename F>
  inline auto asyncDelay(F&& callback, qi::Duration delay)
      -> decltype(detail::asyncMaybeActor(std::forward<F>(callback), delay))
  {
    return detail::asyncMaybeActor(std::forward<F>(callback), delay);
  }
  template <typename F>
  inline auto asyncAt(F&& callback, qi::SteadyClockTimePoint timepoint)
      -> decltype(qi::getEventLoop()->asyncAt(std::forward<F>(callback), timepoint))
  {
    return qi::getEventLoop()->asyncAt(std::forward<F>(callback), timepoint);
  }
  template <typename F>
  inline auto async(F&& callback)
      -> decltype(asyncDelay(std::forward<F>(callback), qi::Duration(0)))
  {
    return asyncDelay(std::forward<F>(callback), qi::Duration(0));
  }

#ifdef DOXYGEN
  /// @deprecated since 2.5
  template<typename R, typename Func, typename ArgTrack>
  QI_API_DEPRECATED qi::Future<R> async(const Func& f, const ArgTrack& toTrack, ...);
#else
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                                                   \
  template <typename R, typename AF, typename ARG0 comma ATYPEDECL>                                         \
  inline QI_API_DEPRECATED Future<R> async(const AF& fun, const ARG0& arg0 comma ADECL, qi::Duration delay = qi::Duration(0)) \
  {                                                                                                         \
    return detail::asyncMaybeActor(qi::bind(fun, arg0 comma AUSE), delay);            \
  }                                                                                                         \
  template <typename R, typename AF, typename ARG0 comma ATYPEDECL>                                         \
  inline QI_API_DEPRECATED Future<R> async(const AF& fun, const ARG0& arg0 comma ADECL, qi::SteadyClockTimePoint timepoint)   \
  {                                                                                                         \
    return detail::asyncMaybeActor(qi::bind(fun, arg0 comma AUSE), timepoint);        \
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

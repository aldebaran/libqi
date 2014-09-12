#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_EXECUTION_CONTEXT_HPP_
#define _QI_EXECUTION_CONTEXT_HPP_

#include <boost/function.hpp>
#include <qi/clock.hpp>
#include <qi/api.hpp>

namespace qi
{

template <typename T>
class Future;

class QI_API ExecutionContext
{
public:
  virtual ~ExecutionContext() {}

  /// post a callback to be executed as soon as possible
  virtual void post(const boost::function<void()>& callback) = 0;
  /// call a callback asynchronously to be executed on tp
  virtual qi::Future<void> async(const boost::function<void()>& callback,
      qi::SteadyClockTimePoint tp) = 0;
  /// call a callback asynchronously to be executed in delay
  virtual qi::Future<void> async(const boost::function<void()>& callback,
      qi::Duration delay) = 0;
  /// call a callback asynchronously to be executed in delay
  template <typename R>
  typename boost::enable_if_c<!boost::is_same<R, void>::value,
                              qi::Future<R> >::type
      async(const boost::function<R()>& callback, qi::Duration delay);
  /// call a callback asynchronously to be executed on tp
  template <typename R>
  typename boost::enable_if_c<!boost::is_same<R, void>::value,
                              qi::Future<R> >::type
      async(const boost::function<R()>& callback, qi::SteadyClockTimePoint tp);

  /// return true if the current thread is in this context
  virtual bool isInThisContext() = 0;
};

}

#include <qi/detail/future_fwd.hpp>

namespace qi
{

namespace detail
{

template <typename T>
class DelayedPromise: public Promise<T>
{
public:
  DelayedPromise() {}
  void setup(boost::function<void (qi::Promise<T>)> cancelCallback, FutureCallbackType async = FutureCallbackType_Async)
  {
    Promise<T>::setup(cancelCallback, async);
  }
};

template<typename R> void call_and_set(qi::Promise<R> p, boost::function<R()> f)
{
  try
  {
    p.setValue(f());
  }
  catch (const std::exception& e)
  {
    p.setError(e.what());
  }
  catch(...)
  {
    p.setError("unknown exception");
  }
}
template<typename R> void check_canceled(qi::Future<void> f, qi::Promise<R> p)
{
  if (f.wait() == FutureState_Canceled)
    p.setCanceled();
  // Nothing to do for other states.
}

}

template <typename R>
typename boost::enable_if_c<!boost::is_same<R, void>::value,
                            qi::Future<R> >::type
    ExecutionContext::async(const boost::function<R()>& callback,
                            qi::Duration delay)
{
  detail::DelayedPromise<R> promise;
  qi::Future<void> f = async(boost::function<void()>(boost::bind(
                                 detail::call_and_set<R>, promise, callback)),
                             delay);
  promise.setup(
      boost::bind(&detail::futureCancelAdapter<void>,
                  boost::weak_ptr<detail::FutureBaseTyped<void> >(f.impl())),
      FutureCallbackType_Sync);
  f.connect(boost::bind(&detail::check_canceled<R>, _1, promise));
  return promise.future();
}

template <typename R>
typename boost::enable_if_c<!boost::is_same<R, void>::value,
                            qi::Future<R> >::type
    ExecutionContext::async(const boost::function<R()>& callback,
                            qi::SteadyClockTimePoint tp)
{
  detail::DelayedPromise<R> promise;
  qi::Future<void> f = async(boost::function<void()>(boost::bind(
                                 detail::call_and_set<R>, promise, callback)),
                             tp);
  promise.setup(
      boost::bind(&detail::futureCancelAdapter<void>,
                  boost::weak_ptr<detail::FutureBaseTyped<void> >(f.impl())),
      FutureCallbackType_Sync);
  f.connect(boost::bind(&detail::check_canceled<R>, _1, promise));
  return promise.future();
}

}

#endif

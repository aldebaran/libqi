#pragma once
/**
**  Copyright (C) 2012-2016 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_DETAIL_ASYNC_HXX_
#define _QI_DETAIL_ASYNC_HXX_

#include <qi/async.hpp>
#include <functional>

namespace qi
{
namespace detail
{
  template <typename F>
  inline auto asyncMaybeActor(F&& cb, qi::Duration delay) ->
      typename std::enable_if<detail::IsAsyncBind<F>::value, typename std::decay<decltype(cb())>::type>::type
  {
    if (delay != qi::Duration::zero())
      return qi::getEventLoop()->asyncDelay(cb, delay).unwrap();
    else
      return cb();
  }

  template <typename F>
  inline auto asyncMaybeActor(F&& cb, qi::Duration delay) ->
      typename std::enable_if<!detail::IsAsyncBind<F>::value,
               qi::Future<typename std::decay<decltype(cb())>::type>>::type
  {
    return qi::getEventLoop()->asyncDelay(cb, delay);
  }

  template <typename F>
  inline auto asyncMaybeActor(F&& cb, qi::SteadyClockTimePoint timepoint) ->
      typename std::enable_if<detail::IsAsyncBind<F>::value, typename std::decay<decltype(cb())>::type>::type
  {
    return qi::getEventLoop()->asyncAt(cb, timepoint).unwrap();
  }
  template <typename F>
  inline auto asyncMaybeActor(F&& cb, qi::SteadyClockTimePoint timepoint) ->
      typename std::enable_if<!detail::IsAsyncBind<F>::value,
               qi::Future<typename std::decay<decltype(cb())>::type>>::type
  {
    return qi::getEventLoop()->asyncAt(cb, timepoint);
  }

  // this function is used by qilang generation to call something regardless of if it's an actor or not
  // it is very specific to qilang's generated code, I don't think it's a good idea to use it elsewhere
  // this function may or may not return a future (the other overload does not)
  template <typename F, typename Arg0, typename... Args>
  auto invokeMaybeActor(F&& cb, Arg0* arg0, Args&&... args) ->
      typename std::enable_if<std::is_base_of<Actor, typename std::decay<Arg0>::type>::value,
                decltype(tryUnwrap(arg0->async(boost::bind(cb, arg0, std::forward<Args>(args)...)), 0))>::type
  {
    // this is an actor, we must async to strand the call
    return tryUnwrap(arg0->async(boost::bind(cb, arg0, std::forward<Args>(args)...)), 0);
  }
  template <typename F, typename Arg0, typename... Args>
  auto invokeMaybeActor(F&& cb, Arg0* arg0, Args&&... args) ->
      typename std::enable_if<!std::is_base_of<Actor, typename std::decay<Arg0>::type>::value,
               typename std::decay<decltype((arg0->*cb)(std::forward<Args>(args)...))>::type>::type
  {
    QI_ASSERT(arg0 && "the pointer on which the member is called is null");
    return (arg0->*cb)(std::forward<Args>(args)...);
  }
} // detail

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

namespace detail
{
  template<typename T>
  void tryCancel(Future<T>& fut, const char* errorMsg)
  {
    try
    {
      // This condition is racy, but the goal is to avoid useless logs
      // (in the catch clauses).
      if (fut.isRunning())
      {
        fut.cancel();
      }
    }
    catch (const std::runtime_error& e)
    {
      qiLogVerbose("qi.Future") << errorMsg << "detail=" << e.what();
    }
    catch (...)
    {
      qiLogVerbose("qi.Future") << errorMsg << "No detail.";
    }
  }
}

/// Cancels the future when the timeout expires.
///
/// The output future is the same as the input one, to allow functional
/// composition.
template<typename T, typename Duration>
Future<T> cancelOnTimeout(Future<T> fut, Duration timeout)
{
  auto cancelFut = asyncDelay(
      [=]() mutable {
        detail::tryCancel(fut, "cancelOnTimeout: timeout task failed to cancel the running task: ");
      },
      timeout);
  fut.then([=](const Future<T>&) mutable {
    detail::tryCancel(cancelFut, "cancelOnTimeout: running task failed to cancel the timeout task");
  });
  return fut;
}

} // qi
#endif  // _QI_DETAIL_ASYNC_HXX_

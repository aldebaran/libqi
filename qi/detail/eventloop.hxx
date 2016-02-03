#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_DETAIL_EVENTLOOP_HXX_
#define _QI_DETAIL_EVENTLOOP_HXX_

#include <qi/detail/future_fwd.hpp>
#include <qi/actor.hpp>

namespace qi
{

template <typename R>
void nullConverter(void*, R&)
{}

template <typename R>
Future<R> EventLoop::async(const boost::function<R()>& callback,
                                  uint64_t usDelay)
{
  return async(callback, qi::MicroSeconds(usDelay));
}

namespace detail
{
  template <typename F>
  auto asyncMaybeActor(F&& cb, qi::Duration delay) ->
      typename std::enable_if<detail::IsAsyncBind<F>::value, typename std::decay<decltype(cb())>::type>::type
  {
    if (delay != qi::Duration::zero())
      return qi::getEventLoop()->asyncDelay(cb, delay).unwrap();
    else
      return cb();
  }
  template <typename F>
  auto asyncMaybeActor(F&& cb, qi::Duration delay) ->
      typename std::enable_if<!detail::IsAsyncBind<F>::value,
               qi::Future<typename std::decay<decltype(cb())>::type>>::type
  {
    return qi::getEventLoop()->asyncDelay(cb, delay);
  }
  template <typename F>
  auto asyncMaybeActor(F&& cb, qi::SteadyClockTimePoint timepoint) ->
      typename std::enable_if<detail::IsAsyncBind<F>::value, typename std::decay<decltype(cb())>::type>::type
  {
    return qi::getEventLoop()->asyncAt(cb, timepoint).unwrap();
  }
  template <typename F>
  auto asyncMaybeActor(F&& cb, qi::SteadyClockTimePoint timepoint) ->
      typename std::enable_if<!detail::IsAsyncBind<F>::value,
               qi::Future<typename std::decay<decltype(cb())>::type>>::type
  {
    return qi::getEventLoop()->asyncAt(cb, timepoint);
  }

  // sfinae to try to call unwrap on the future
  template <typename F>
  auto tryUnwrap(const F& future, int) -> decltype(future.unwrap())
  {
    return future.unwrap();
  }
  template <typename F>
  F tryUnwrap(const F& future, void*)
  {
    return future;
  }

  // this function is used by qilang generation to call something regardless of if it's an actor or not
  // it is very specific to qilang's generated code, I don't think it's a good idea to use it elsewhere
  // this function may or may not return a future (the other overload does not)
  template <typename F, typename Arg0, typename... Args>
  auto invokeMaybeActor(F&& cb, Arg0* arg0, Args&&... args) ->
      typename std::enable_if<std::is_base_of<Actor, typename std::decay<Arg0>::type>::value,
               decltype(tryUnwrap(qi::async(qi::bind(cb, arg0, std::forward<Args>(args)...)), 0))>::type
  {
    // this is an actor, we must async to strand the call
    return tryUnwrap(qi::async(qi::bind(cb, arg0, std::forward<Args>(args)...)), 0);
  }
  template <typename F, typename Arg0, typename... Args>
  auto invokeMaybeActor(F&& cb, Arg0* arg0, Args&&... args) ->
      typename std::enable_if<!std::is_base_of<Actor, typename std::decay<Arg0>::type>::value,
               typename std::decay<decltype((arg0->*cb)(std::forward<Args>(args)...))>::type>::type
  {
    return (arg0->*cb)(std::forward<Args>(args)...);
  }
}

}

#endif

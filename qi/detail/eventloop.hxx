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
}

}

#endif

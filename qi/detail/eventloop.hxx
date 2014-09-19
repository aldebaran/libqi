#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_DETAILS_EVENTLOOP_HXX_
#define _QI_DETAILS_EVENTLOOP_HXX_

#include <qi/detail/future_fwd.hpp>
#include <qi/actor.hpp>

namespace qi
{

  template<typename R> void nullConverter(void*, R&) {}
  template<typename R> Future<R> EventLoop::async(const boost::function<R()>& callback, uint64_t usDelay)
  {
    return async(callback, qi::MicroSeconds(usDelay));
  }

  namespace detail
  {
    template <
        typename R,
        typename ARG0,
        typename boost::enable_if<
            boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
            int>::type>
    inline Future<R> asyncMaybeActor(
        const ARG0& arg0, const boost::function<R()>& cb,
        qi::Duration delay)
    {
      return detail::Unwrap<ARG0>::unwrap(arg0)->strand()->async(cb, delay);
    }
    template <
        typename R,
        typename ARG0,
        typename boost::disable_if<
            boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
            int>::type>
    inline Future<R> asyncMaybeActor(
        const ARG0& arg0, const boost::function<R()>& cb,
        qi::Duration delay)
    {
      return qi::getEventLoop()->async(cb, delay);
    }
    template <
        typename R,
        typename ARG0,
        typename boost::enable_if<
            boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
            int>::type>
    inline Future<R> asyncMaybeActor(
        const ARG0& arg0, const boost::function<R()>& cb,
        qi::SteadyClockTimePoint timepoint)
    {
      return detail::Unwrap<ARG0>::unwrap(arg0)->strand()
        ->async(cb, timepoint);
    }
    template <
        typename R,
        typename ARG0,
        typename boost::disable_if<
            boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
            int>::type>
    inline Future<R> asyncMaybeActor(
        const ARG0& arg0, const boost::function<R()>& cb,
        qi::SteadyClockTimePoint timepoint)
    {
      return qi::getEventLoop()->async(cb, timepoint);
    }
  }

}

#endif

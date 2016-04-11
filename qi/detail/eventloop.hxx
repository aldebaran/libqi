#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_DETAIL_EVENTLOOP_HXX_
#define _QI_DETAIL_EVENTLOOP_HXX_

#include <qi/detail/future_fwd.hpp>

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
} // qi

#endif

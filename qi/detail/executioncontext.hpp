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
  /// post a callback to be executed in delay
  virtual qi::Future<void> async(const boost::function<void()>& callback,
      qi::Duration delay) = 0;

  /// return true if the current thread is in this context
  virtual bool isInThisContext() = 0;
};

}

#endif

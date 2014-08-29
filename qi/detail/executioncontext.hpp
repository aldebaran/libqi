#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_EXECUTION_CONTEXT_HPP_
#define _QI_EXECUTION_CONTEXT_HPP_

#include <boost/function.hpp>
#include <qi/api.hpp>

namespace qi
{

class QI_API ExecutionContext
{
public:
  virtual ~ExecutionContext() {}
  /// post a callback to be executed as soon as possible
  virtual void post(const boost::function<void()>& callback) = 0;
  /// return true if the current thread is in this context
  virtual bool isInThisContext() = 0;
};

}

#endif

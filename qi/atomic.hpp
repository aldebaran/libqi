/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once

#ifndef _LIBQI_QI_ATOMIC_HPP_
#define _LIBQI_QI_ATOMIC_HPP_

# include <qi/config.hpp>

namespace qi
{
  template <typename T>
  class QI_API atomic
  {
  public:
    atomic()
      : _value(0)
    {
    }

    atomic(T value)
      : _value(value)
    {
    }

    /* prefix operators */
    T operator++();
    T operator--();

    T operator*()
    {
      return _value;
    }

  private:
    T _value;
  };
}

#endif // _LIBQI_QI_ATOMIC_HPP_

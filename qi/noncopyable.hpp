/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#pragma once
#ifndef _LIBQI_QI_NONCOPYABLE_HPP_
#define _LIBQI_QI_NONCOPYABLE_HPP_

namespace qi {

  class noncopyable
  {
  protected:
    //object not directly instantiable
    noncopyable()  {}
    ~noncopyable() {}

  private:
    //private copy
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
  };
}

#endif  // _LIBQI_QI_NONCOPYABLE_HPP_

#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MPL_HPP_
#define _QI_MPL_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace qi
{
namespace detail
{

template <typename T>
struct Unwrap
{
  typedef T type;
  static T* unwrap(T& v)
  {
    return &v;
  }
};
template <typename T>
struct Unwrap<T*>
{
  typedef T type;
  static T* unwrap(T* v)
  {
    return v;
  }
};
template <typename T>
struct Unwrap<boost::shared_ptr<T> >
{
  typedef T type;
  static T* unwrap(boost::shared_ptr<T> v)
  {
    return *v.get();
  }
};
template <typename T>
struct Unwrap<boost::weak_ptr<T> >
{
  typedef T type;
  static T* unwrap(boost::weak_ptr<T> v)
  {
    return v.lock().get();
  }
};

}
}

#endif

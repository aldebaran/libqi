#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_MPL_HPP_
#define _QI_MPL_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <type_traits>

namespace qi
{
namespace detail
{

template <typename T>
struct UnwrapImpl
{
  using type = T;
  static T* unwrap(T& v)
  {
    return &v;
  }
};
template <typename T>
struct UnwrapImpl<T*>
{
  using type = T;
  static T* unwrap(T* v)
  {
    return v;
  }
};
template <typename T>
struct UnwrapImpl<boost::shared_ptr<T> >
{
  using type = T;
  static T* unwrap(boost::shared_ptr<T> v)
  {
    return v.get();
  }
};
template <typename T>
struct UnwrapImpl<boost::weak_ptr<T> >
{
  using type = T;
  static T* unwrap(boost::weak_ptr<T> v)
  {
    return v.lock().get();
  }
};

template <typename T>
struct Unwrap : public UnwrapImpl<typename std::remove_cv<typename std::remove_reference<T>::type>::type>
{};

}
}

#endif

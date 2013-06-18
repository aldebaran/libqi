#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_FUNCTIONTYPEFACTORY_HPP_
#define _QITYPE_FUNCTIONTYPEFACTORY_HPP_

#include <qitype/functiontype.hpp>

namespace qi
{

  /*** @return a AnyFunction wrapping func.
  * func can be:
  * - a boost::bind object
  * - a boost::function
  * - a function pointer
  * - a member function pointer
  *
  */
  template<typename F> AnyFunction makeAnyFunction(F func);
  /// @return a AnyFunction binding \p instance to member function \p func
  template<typename F, typename C> AnyFunction makeAnyFunction(F func, C instance);
}

#include <qitype/details/functiontypefactory.hxx>


#endif  // _QITYPE_FUNCTIONTYPEFACTORY_HPP_

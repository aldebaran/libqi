#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_METHODTYPEFACTORY_HPP_
#define _QITYPE_METHODTYPEFACTORY_HPP_

namespace qi
{
  template<typename T> MethodType* methodTypeOf();

  /** Construct a GenericMethod from a pointer to method or function.
   * If argument is a pointer to function, the first call argument
   * representing the class instance will be dropped.
  */
  template<typename M>
  GenericMethod makeGenericMethod(const M& method);

  /** Similar to makeGenericMethod(), but expects a non-member
   * function, and do not drop the first argument.
  */
  template<typename M>
  GenericMethod makeGenericMethodFromFunction(const M& method);
}

#include <qitype/details/methodtypefactory.hxx>
#endif

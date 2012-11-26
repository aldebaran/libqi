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

  template<typename M>
  GenericMethod makeGenericMethod(const M& method);
}

#include <qitype/details/methodtypefactory.hxx>
#endif

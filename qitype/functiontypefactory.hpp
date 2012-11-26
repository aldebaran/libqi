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

  template<typename F> GenericFunction makeGenericFunction(F func);

  /// @return a GenericFunction obtained by binding a class instance to a member function
  template<typename O, typename F> GenericFunction makeGenericFunction(O o, F f);
}

#include <qitype/details/functiontypefactory.hxx>


#endif

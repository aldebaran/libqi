#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_FWD_HPP_
#define _QITYPE_FWD_HPP_

#include <boost/shared_ptr.hpp>

namespace qi
{
  class TypeInfo;

  class TypeInterface;
  class IntTypeInterface;
  class FloatTypeInterface;
  class StringTypeInterface;
  class RawTypeInterface;
  class PointerTypeInterface;
  class ListTypeInterface;
  class MapTypeInterface;
  class StructTypeInterface;
  class DynamicTypeInterface;

  class AnyReference;
  class AnyReference;
  class GenericValue;
  class AnyIterator;
  class GenericListPtr;
  class GenericMapPtr;
  class GenericValue;
  class GenericObject;
  class AutoAnyReference;
  class Manageable;
  typedef boost::shared_ptr<GenericObject> AnyObject;
}

#endif  // _QITYPE_FWD_HPP_

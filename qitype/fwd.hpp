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
  class Type;
  class TypeInt;
  class TypeFloat;
  class TypeString;
  class TypeRaw;
  class TypePointer;
  class TypeList;
  class TypeMap;
  class TypeTuple;
  class TypeDynamic;
  class GenericValuePtr;
  class GenericListPtr;
  class GenericMapPtr;
  class GenericValue;
  class GenericObject;
  class AutoGenericValuePtr;
  class Manageable;
  typedef boost::shared_ptr<GenericObject> ObjectPtr;
}

#endif

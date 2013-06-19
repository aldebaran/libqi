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

  class AutoAnyReference;

  class AnyReference;
  class AnyValue;
  class AnyIterator;

  class GenericListPtr;
  class GenericMapPtr;
  class Manageable;

  class GenericObject;
  typedef boost::shared_ptr<GenericObject> AnyObject;

  class Signature;

  //warning update the C enum when updating this one.
  enum TypeKind
  {
    TypeKind_Void     = 0,
    TypeKind_Int      = 1,
    TypeKind_Float    = 2,
    TypeKind_String   = 3,
    TypeKind_List     = 4,
    TypeKind_Map      = 5,
    TypeKind_Object   = 6,
    TypeKind_Pointer  = 7,
    TypeKind_Tuple    = 8,
    TypeKind_Dynamic  = 9,
    TypeKind_Raw      = 10,
    TypeKind_Unknown  = 11,
    TypeKind_Iterator = 12,
  };

}

#endif  // _QITYPE_FWD_HPP_

#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_FWD_HPP_
#define _QI_TYPE_FWD_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <vector>

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
  using AnyReferenceVector = std::vector<AnyReference>;

  class AnyValue;
  using AnyValueVector = std::vector<AnyValue>;

  class AnyIterator;

  class Manageable;

  class Empty;
  class Proxy;
  template<typename T=Empty> class Object;
  using AnyObject = Object<Empty>;
  template<typename T=Empty> class WeakObject;
  using AnyWeakObject = WeakObject<Empty>;

  class GenericObject;

  class Signature;

  //warning update the C enum when updating this one.
  enum TypeKind
  {
    TypeKind_Unknown  = 0,
    TypeKind_Void     = 1,
    TypeKind_Int      = 2,
    TypeKind_Float    = 3,
    TypeKind_String   = 4,
    TypeKind_List     = 5,
    TypeKind_Map      = 6,
    TypeKind_Object   = 7,
    TypeKind_Pointer  = 8,
    TypeKind_Tuple    = 9,
    TypeKind_Dynamic  = 10,
    TypeKind_Raw      = 11,
    TypeKind_Iterator = 13,
    TypeKind_Function = 14,
    TypeKind_Signal   = 15,
    TypeKind_Property = 16,
    TypeKind_VarArgs  = 17,
  };

}

#endif  // _QITYPE_FWD_HPP_

#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_GENERICVALUE_HPP_
#define _QIMESSAGING_GENERICVALUE_HPP_

#include <qimessaging/type.hpp>
namespace qi {

class GenericList;
class GenericMap;
class GenericObject;
class GenericList;


/** Class that holds any value, with informations to manipulate it.
 *  Operator = makes a shallow copy.
 *
 */
class QIMESSAGING_API GenericValue
{
public:
  GenericValue();

  /// Convert GenericValue to concrete type. Returns the value and a
  /// bool that is true if value is a copy and must be deleted.
  template<typename T> std::pair<const T*, bool> to() const;

  /// @return the pair (convertedValue, trueIfCopiedAndNeedsDestroy)
  std::pair<GenericValue, bool> convert(Type* targetType) const;
  // kind-based converter
  std::pair<GenericValue, bool> convert2(Type* targetType) const;
  /// Helper function that converts and always clone
  GenericValue convertCopy(Type* targetType) const;
  GenericValue clone() const;
  void serialize(ODataStream& os) const;
  std::string signature() const;
  void destroy();
  Type::Kind kind() const;

  int64_t asInt() const;
  float   asFloat() const;
  double  asDouble() const;

  std::string asString() const;

  GenericList asList() const;
  GenericMap  asMap() const;
  GenericObject asObject() const;

  template<typename T, Type::Kind k> T as() const;
  template<typename T> T as() const;

  void*   value;
  Type*   type;
};

/// Convert any value to the correct associated Value
template<typename T> GenericValue toValue(const T& v);


/** Generates GenericValue from everything transparently.
 * To be used as type of meta-function call argument
 *
 *  Example:
 *    void metaCall(ValueGen arg1, ValueGen arg2);
 *  can be called with any argument type:
 *    metaCall("foo", 12);
 */
class AutoGenericValue: public GenericValue
{
public:
  AutoGenericValue ();
  AutoGenericValue(const AutoGenericValue & b);

  template<typename T> AutoGenericValue(const T& ptr);
};


}

#include <qimessaging/details/genericvalue.hxx>

#endif  // _QIMESSAGING_GENERICVALUE_HPP_

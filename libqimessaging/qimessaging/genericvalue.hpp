#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_GENERICVALUE_HPP_
#define _QIMESSAGING_GENERICVALUE_HPP_

#include <qimessaging/type.hpp>
namespace qi {

/** Class that holds any value, with informations to manipulate it.
 *  Operator = makes a shallow copy.
 *
 */
class QIMESSAGING_API GenericValue
{
public:
  GenericValue() : value(0), type(0) {}
  void*          value;
  Type*      type;

  // TODO Make bouncers to type methods here
  /// Convert GenericValue to concrete type. Returns the value and a
  /// bool that is true if value is a copy and must be deleted.
  template<typename T> std::pair<const T*, bool> as() const;
  GenericValue convert(Type& targetType) const;
  GenericValue clone() const;
  void serialize(ODataStream& os) const { if(type) type->serialize(os, value);}
  std::string signature() const
  {
    if (!type)
      return "";
    else
      return type->signature();
  }
  void destroy()
  {
    if (type && value)
      type->destroy(value);
  }
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

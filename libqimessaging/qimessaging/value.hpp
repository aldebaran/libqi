/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once

#ifndef _QI_MESSAGING_VALUE_HH_
#define _QI_MESSAGING_VALUE_HH_

#include <qimessaging/type.hpp>
namespace qi {

/** Class that holds any value, with informations to manipulate it.
 *  Operator = makes a shallow copy.
 *
 */
class QIMESSAGING_API Value
{
public:
  Value() : value(0), type(0) {}
  void*          value;
  Type*      type;

  // TODO Make bouncers to type methods here
  /// Convert Value to concrete type. Returns the value and a
  /// bool that is true if value is a copy and must be deleted.
  template<typename T> std::pair<const T*, bool> as() const;
  Value convert(Type& targetType) const;
  Value clone() const;
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
template<typename T> Value toValue(const T& v);


/** Generates Value from everything transparently.
 * To be used as type of meta-function call argument
 *
 *  Example:
 *    void metaCall(ValueGen arg1, ValueGen arg2);
 *  can be called with any argument type:
 *    metaCall("foo", 12);
 */
class AutoValue: public Value
{
public:
  AutoValue();
  AutoValue(const AutoValue& b);

  template<typename T> AutoValue(const T& ptr);
};


}

#include <qimessaging/details/value.hxx>

#endif

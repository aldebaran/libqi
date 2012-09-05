/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once

#ifndef _QI_MESSAGING_METAVALUE_HH_
#define _QI_MESSAGING_METAVALUE_HH_

#include <qimessaging/metatype.hpp>
namespace qi {

/** Class that holds any value, with informations to manipulate it.
 *  Operator = makes a shallow copy.
 *
 */
class QIMESSAGING_API MetaValue
{
public:
  MetaValue() : value(0), type(0) {}
  void*          value;
  MetaType*      type;

  // TODO Make bouncers to type methods here
  /// Convert MetaValue to concrete type. Returns the value and a
  /// bool that is true if value is a copy and must be deleted.
  template<typename T> std::pair<const T*, bool> as() const;
  MetaValue convert(MetaType& targetType) const;
  MetaValue clone() const;
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

/// Convert any value to the correct associated MetaValue
template<typename T> MetaValue toMetaValue(const T& v);


/** Generates MetaValue from everything transparently.
 * To be used as type of meta-function call argument
 *
 *  Example:
 *    void metaCall(MetaValueGen arg1, MetaValueGen arg2);
 *  can be called with any argument type:
 *    metaCall("foo", 12);
 */
class AutoMetaValue: public MetaValue
{
public:
  AutoMetaValue();
  AutoMetaValue(const AutoMetaValue& b);

  template<typename T> AutoMetaValue(const T& ptr);
};

/** This class can be used to convert the return value of an arbitrary function
 * into a MetaValue. It handles functions returning void.
 *
 *  Usage:
 *    MetaValueCopy val;
 *    val(), functionCall(arg);
 *
 *  in val(), parenthesis are useful to avoid compiler warning "val not used" when handling void.
 */
class MetaValueCopy: public MetaValue
{
public:
  template<typename T> void operator,(const T& any);
  MetaValueCopy &operator()() { return *this; }
};


qi::ODataStream& operator << (qi::ODataStream& od, const MetaValue& mv);


}

QI_NO_METATYPE(qi::MetaValue)
QI_NO_METATYPE(std::vector<qi::MetaValue>)
#include <qimessaging/metavalue.hxx>

#endif

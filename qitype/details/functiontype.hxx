#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_FUNCTIONTYPE_HXX_
#define _QITYPE_DETAILS_FUNCTIONTYPE_HXX_


#include <qitype/genericvalue.hpp>
#include <qitype/details/bindtype.hxx>

namespace qi
{
  inline CallableType::CallableType()
  : _resultType(0)
  {
  }

  inline Type* CallableType::resultType()
  {
    return _resultType;
  }

  inline const std::vector<Type*>& CallableType::argumentsType()
  {
    return _argumentsType;
  }

  inline GenericValuePtr GenericFunction::operator()(const std::vector<GenericValuePtr>& args)
  {
    return call(args);
  }

  inline GenericFunction::GenericFunction()
  : type(0), value(0) {}

  inline GenericFunction::GenericFunction(const GenericFunction& b)
  {
    type = b.type;
    value = type?type->clone(b.value):0;
  }

  inline GenericFunction& GenericFunction::operator=(const GenericFunction& b)
  {
    this->~GenericFunction();
    type = b.type;
    value = type?type->clone(b.value):0;
    return *this;
  }

  inline GenericFunction::~GenericFunction()
  {
    if (type)
      type->destroy(value);
  }

  inline GenericValuePtr GenericFunction::call(const std::vector<GenericValuePtr>& args)
  {
    return type->call(value, args);
  }



} // namespace qi
#endif  // _QITYPE_DETAILS_FUNCTIONTYPE_HXX_

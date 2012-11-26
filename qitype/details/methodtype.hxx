#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_METHODTYPE_HXX_
#define _QITYPE_DETAILS_METHODTYPE_HXX_


namespace qi
{
  inline GenericMethod::GenericMethod()
  : type(0), value(0) {}

  inline GenericMethod::GenericMethod(const GenericMethod& b)
  {
    type = b.type;
    value = type?type->clone(b.value):0;
  }

  inline GenericMethod::~GenericMethod()
  {
    if (type)
      type->destroy(value);
  }

  inline GenericMethod& GenericMethod::operator = (const GenericMethod& b)
  {
    this->~GenericMethod();
    type = b.type;
    value = type?type->clone(b.value):0;
    return *this;
  }

  inline GenericFunction GenericMethod::toGenericFunction()
  {
    GenericFunction result;
    result.type = type->toFunctionType();
    result.value = type->clone(value);
    return result;
  }
} // namespace qi

#endif  // _QITYPE_DETAILS_METHODTYPE_HXX_

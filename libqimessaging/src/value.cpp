/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/details/dynamicvalue.hpp>
#include <cassert>

namespace qi {
  namespace detail {

  void DynamicValue::clear() {
    switch(type) {
    case String:
      delete data.str;
      break;
    case List:
      delete data.list;
      break;
    case Map:
      delete data.map;
      break;
    default:
      break;
    }
    data.ptr = 0;
    type = Invalid;
  }

  DynamicValue::~DynamicValue()
  {
    clear();
  }
  }
}


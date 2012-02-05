/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qimessaging/object.hpp>

namespace qi {

  Object::Object()
    : _meta(new MetaObject())
  {
  }

  Object::~Object() {
    delete _meta;
  }

  MetaObject &Object::metaObject() {
    return *_meta;
  }

};

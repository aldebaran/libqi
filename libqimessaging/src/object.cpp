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

  MetaMethod::MetaMethod(const std::string &name, const std::string &sig, const qi::Functor *functor)
    : _name(name),
      _signature(sig),
      _functor(functor)
  {
  }

  void Object::xAdvertiseService(const std::string &name, const std::string& signature, const qi::Functor* functor) {
    MetaMethod mm(name, signature, functor);
    _meta->_methods[name] = mm;
  }

  void Object::metaCall(const std::string &method, const std::string &sig, DataStream &in, DataStream &out)
  {
    MetaMethod &mm = metaObject()._methods[method];
    mm._functor->call(in, out);
  }

};

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
    advertiseMethod("__metaobject", this, &Object::metaObject);
  }

  Object::~Object() {
    delete _meta;
  }

  MetaObject &Object::metaObject() {
    return *_meta;
  }

  MetaMethod::MetaMethod()
    : _signature("")
    , _functor(0)
    , _idx(0)
  {
  }

  MetaMethod::MetaMethod(const std::string &sig, const qi::Functor *functor)
    : _signature(sig)
    , _functor(functor)
    , _idx(0)
  {
  }

  unsigned int Object::xAdvertiseMethod(const std::string& signature, const qi::Functor* functor) {
    MetaMethod mm(signature, functor);
    unsigned int idx = _meta->_methodsNumber++;
    mm._idx = idx;
    _meta->_methods.push_back(mm);
    _meta->_methodsNameToIdx[signature] = idx;
    return idx;
  }

  void Object::metaCall(unsigned int method, const FunctorParameters &in, qi::FunctorResult out)
  {
    assert(method < _meta->_methods.size());
    MetaMethod *mm = &(_meta->_methods[method]);
    if (mm->_functor)
      mm->_functor->call(in, out);
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaMethod &meta) {
    stream << meta._signature;
    stream << meta._idx;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaMethod &meta) {
    stream >> meta._signature;
    stream >> meta._idx;
    return stream;
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaObject &meta) {
    stream << meta._methodsNameToIdx;
    stream << meta._methods;
    stream << meta._methodsNumber;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaObject &meta) {
    stream >> meta._methodsNameToIdx;
    stream >> meta._methods;
    stream >> meta._methodsNumber;
    return stream;
  }

};

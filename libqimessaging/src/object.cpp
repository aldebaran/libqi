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
    : _name("")
    , _signature("")
    , _functor(0)
    , _idx(0)
  {
  }

  MetaMethod::MetaMethod(const std::string &name, const std::string &sig, const qi::Functor *functor)
    : _name(name)
    , _signature(sig)
    , _functor(functor)
    , _idx(0)
  {
  }

  unsigned int Object::xAdvertiseMethod(const std::string &name, const std::string& signature, const qi::Functor* functor) {
    MetaMethod mm(name, signature, functor);
    unsigned int idx = _meta->_methodsNumber++;
    mm._idx = idx;
    _meta->_methods[name] = mm;
    _meta->_methodsTable.push_back(&(_meta->_methods[name]));
    return idx;
  }

  void Object::metaCall(const std::string &method, const std::string &sig, DataStream &in, DataStream &out)
  {
    metaCall(metaObject()._methods[method]._idx, sig, in, out);
  }

  void Object::metaCall(unsigned int method, const std::string &sig, DataStream &in, DataStream &out)
  {
    //TODO: correctly handle failure
    assert(method < metaObject()._methodsTable.size());
    MetaMethod *mm = metaObject()._methodsTable[method];
    if (mm->_functor)
      mm->_functor->call(in, out);
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaMethod &meta) {
    stream << meta._name;
    stream << meta._signature;
    stream << meta._idx;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaMethod &meta) {
    stream >> meta._name;
    stream >> meta._signature;
    stream >> meta._idx;
    return stream;
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaObject &meta) {
    stream << meta._methods;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaObject &meta) {
    stream >> meta._methods;
    return stream;
  }


};

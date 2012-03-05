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
    _meta->_methods.push_back(mm);
    _meta->_methodsNameToIdx[name] = idx;

    return idx;
  }

  void Object::metaCall(unsigned int method, const std::string &sig, FunctorParameters &in, qi::FunctorResult &out)
  {
    //TODO: correctly handle failure
    assert(method < _meta->_methods.size());
    MetaMethod *mm = &(_meta->_methods[method]);
    if (mm->_functor)
      mm->_functor->call(in, out);
  }

  void Object::metaCall(unsigned int method, const std::string &sig, qi::FunctorParameters &in, qi::FunctorResultPromiseBase *out)
  {
    qi::Buffer        buf;
    qi::FunctorResult fout(&buf);

    assert(method < _meta->_methods.size());
    MetaMethod *mm = &(_meta->_methods[method]);
    if (mm->_functor)
      mm->_functor->call(in, fout);
    out->setValue(fout);
    delete out;
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

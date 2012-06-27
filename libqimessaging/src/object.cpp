/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include "src/metamethod_p.hpp"
#include "src/object_p.hpp"
#include <qimessaging/object.hpp>

namespace qi {

  MetaObject::MetaObject()
  {
    _p = new MetaObjectPrivate();
  }

  MetaObject::MetaObject(const MetaObject &other)
  {
    _p = new MetaObjectPrivate();
    *_p = *(other._p);
  }

  MetaObject& MetaObject::operator=(const MetaObject &other)
  {
    _p = new MetaObjectPrivate();
    *_p = *(other._p);
    return (*this);
  }

  MetaMethod *MetaObject::method(unsigned int id) {
    if (id < _p->_methods.size())
      return &_p->_methods[id];
    return 0;
  }

  MetaMethod *MetaObject::method(unsigned int id) const {
    if (id < _p->_methods.size())
      return &_p->_methods[id];
    return 0;
  }

  inline int MetaObject::methodId(const std::string &name)
  {
    return _p->methodId(name);
  }

  std::vector<MetaMethod> &MetaObject::methods() {
    return _p->methods();
  }

  const std::vector<MetaMethod> &MetaObject::methods() const {
    return _p->methods();
  }
  MetaObject::~MetaObject()
  {
    delete _p;
  }

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

  int Object::xAdvertiseMethod(const std::string &sigret, const std::string& signature, const qi::Functor* functor) {
    std::map<std::string, unsigned int>::iterator it;

    it = _meta->_p->_methodsNameToIdx.find(signature);
    if (it != _meta->_p->_methodsNameToIdx.end())
    {
      unsigned int idx = it->second;
      MetaMethod mm(sigret, signature, functor);
      mm._p->_idx = idx;
      _meta->methods()[idx] = mm;
      qiLogVerbose("qi.Object") << "rebinding method:" << signature;
      return idx;
    }

    MetaMethod mm(sigret, signature, functor);
    unsigned int idx = _meta->_p->_methodsNumber++;
    mm._p->_idx = idx;
    _meta->methods().push_back(mm);
    _meta->_p->_methodsNameToIdx[signature] = idx;
    qiLogVerbose("qi.Object") << "binding method:" << signature;
    return idx;
  }

  void Object::metaCall(unsigned int method, const FunctorParameters &in, qi::FunctorResult out)
  {
    MetaMethod *mm = _meta->method(method);
    if (!mm) {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      qi::Buffer     buf;
      qi::DataStream ds(buf);
      ds << ss.str();
      out.setError(buf);
      return;
    }
    if (mm->_p->_functor)
      mm->_p->_functor->call(in, out);
  }

  bool Object::xMetaCall(const std::string &retsig, const std::string &signature, const FunctorParameters &in, FunctorResult out)
  {
    int methodId = metaObject().methodId(signature);
    if (methodId < 0) {
      qi::Buffer     buf;
      qi::DataStream ds(buf);
      std::stringstream ss;
      ss << "Can't find method: " << signature << std::endl
         << "  Candidate(s):" << std::endl;
      std::vector<qi::MetaMethod>           mml = metaObject().findMethod(qi::signatureSplit(signature)[1]);
      std::vector<qi::MetaMethod>::const_iterator it;

      for (it = mml.begin(); it != mml.end(); ++it) {
        const qi::MetaMethod       &mm = *it;
        ss << "  " << mm.signature();
      }
      qiLogError("object") << ss.str();
      ds << ss.str();
      out.setError(buf);
      return false;
    }
    if (retsig != "v") {
      qi::MetaMethod *mm = metaObject().method(methodId);
      if (!mm) {
        qiLogError("object") << "method " << signature << "(id: " << methodId << ") disapeared mysteriously!";
        return false;
      }
      if (mm->sigreturn() != retsig) {
        qiLogError("object") << "signature mismatch for return value:" << std::endl
                             << "we want: " << retsig << " " << signature << std::endl
                             << "we had:" << mm->sigreturn() << " " << mm->signature();
        return false;
      }
    }
    //TODO: check for metacall to return false when not able to send the answer
    metaCall(methodId, in, out);
    return true;
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaMethod &meta) {
    stream << meta._p->_signature;
    stream << meta._p->_sigret;
    stream << meta._p->_idx;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaMethod &meta) {
    stream >> meta._p->_signature;
    stream >> meta._p->_sigret;
    stream >> meta._p->_idx;
    return stream;
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaObject &meta) {
    stream << meta._p->_methodsNameToIdx;
    stream << meta._p->_methods;
    stream << meta._p->_methodsNumber;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaObject &meta) {
    stream >> meta._p->_methodsNameToIdx;
    stream >> meta._p->_methods;
    stream >> meta._p->_methodsNumber;
    return stream;
  }

  std::vector<qi::MetaMethod> MetaObject::findMethod(const std::string &name)
  {
    return _p->findMethod(name);
  }

};

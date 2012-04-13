/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qimessaging/object.hpp>
#include <boost/algorithm/string/predicate.hpp>

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
    qiLogVerbose("qi::Object") << "binding method:" << signature;
    return idx;
  }

  void Object::metaCall(unsigned int method, const FunctorParameters &in, qi::FunctorResult out)
  {
    if (method > _meta->_methods.size()) {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      qi::Buffer     buf;
      qi::DataStream ds(buf);
      ds << ss.str();
      out.setError(buf);
      return;
    }
    MetaMethod *mm = &(_meta->_methods[method]);
    if (mm->_functor)
      mm->_functor->call(in, out);
  }

  bool Object::xMetaCall(const std::string &signature, const FunctorParameters &in, FunctorResult out)
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
    metaCall(methodId, in, out);
    return true;
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

  std::vector<qi::MetaMethod> MetaObject::findMethod(const std::string &name)
  {
    std::vector<qi::MetaMethod>           ret;
    std::vector<qi::MetaMethod>::iterator it;
    std::string cname(name);
    cname += "::";

    for (it = _methods.begin(); it != _methods.end(); ++it) {
      qi::MetaMethod &mm = *it;
      if (boost::starts_with(mm.signature(), cname))
        ret.push_back(mm);
    }
    return ret;
  }



};

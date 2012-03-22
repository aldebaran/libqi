/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_OBJECT_HPP_
#define _QIMESSAGING_OBJECT_HPP_

#include <map>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/details/makefunctor.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>

namespace qi {

  class QIMESSAGING_API MetaMethod {
  public:
    MetaMethod(const std::string &sig, const qi::Functor *functor);
    MetaMethod();

    const std::string &signature() const { return _signature; }

  protected:
  public:
    std::string        _signature;
    const qi::Functor *_functor;
    unsigned int       _idx;
  };

  QIMESSAGING_API qi::DataStream &operator<<(qi::DataStream &stream, const MetaMethod &meta);
  QIMESSAGING_API qi::DataStream &operator>>(qi::DataStream &stream, MetaMethod &meta);

  class QIMESSAGING_API MetaObject {
  public:
    MetaObject()
      : _methodsNumber(0)
    {
    };

    inline int methodId(const std::string &name) {
      std::map<std::string, unsigned int>::iterator it;
      it = _methodsNameToIdx.find(name);
      if (it == _methodsNameToIdx.end())
        return -1;
      return it->second;
    }

    /*
     * When a member is added, serialization and deserialization
     * operators _MUST_ be updated.
     */
    std::map<std::string, unsigned int> _methodsNameToIdx;
    std::vector<MetaMethod>             _methods;
    unsigned int                        _methodsNumber;
    // std::map<std::string, MethodInfo>   _signals;
    // std::map<std::string, MethodInfo>   _slots;
    // std::map<std::string, PropertyInfo> _properties;
  };

  QIMESSAGING_API qi::DataStream &operator<<(qi::DataStream &stream, const MetaObject &meta);
  QIMESSAGING_API qi::DataStream &operator>>(qi::DataStream &stream, MetaObject &meta);


  class QIMESSAGING_API Object {
  public:
    Object();
    virtual ~Object();

    MetaObject &metaObject();

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method);

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function);


    template <typename RETURN_TYPE>
    qi::Future<RETURN_TYPE> call(const std::string& methodName);
    template <typename RETURN_TYPE, typename P0>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0);
    template <typename RETURN_TYPE, typename P0, typename P1>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7);
    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    qi::Future<RETURN_TYPE> call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8);

    virtual void metaCall(unsigned int method, const FunctorParameters &in, FunctorResult out);

  protected:
    unsigned int xAdvertiseMethod(const std::string& signature, const Functor *functor);

  protected:
    MetaObject *_meta;
  };



template <typename OBJECT_TYPE, typename METHOD_TYPE>
inline unsigned int Object::advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method)
{
  std::string signature(name);
  signature += "::";
  signatureFromObject::value(method, signature);
  return xAdvertiseMethod(signature, makeFunctor(object, method));
}

template <typename FUNCTION_TYPE>
inline unsigned int Object::advertiseMethod(const std::string& name, FUNCTION_TYPE function)
{
  std::string signature(name);
  signature += "::";
  signatureFromObject::value(function, signature);
  return xAdvertiseMethod(signature, makeFunctor(function));
}



};

#include <qimessaging/object.hxx>
#endif  // _QIMESSAGING_OBJECT_HPP_

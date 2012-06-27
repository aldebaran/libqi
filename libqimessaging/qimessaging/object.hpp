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
#include <qimessaging/metamethod.hpp>

namespace qi {

  class MetaObjectPrivate;
  class QIMESSAGING_API MetaObject {
  public:
    MetaObject();
    MetaObject(const MetaObject &other);
    MetaObject& operator=(const MetaObject &other);
    ~MetaObject();

    inline int methodId(const std::string &name);

    std::vector<MetaMethod> &methods();
    const std::vector<MetaMethod> &methods() const;

    MetaMethod *method(unsigned int id);
    MetaMethod *method(unsigned int id) const;

    std::vector<MetaMethod> findMethod(const std::string &name);

    MetaObjectPrivate   *_p;
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
    int xAdvertiseMethod(const std::string &retsig, const std::string& signature, const Functor *functor);
    bool xMetaCall(const std::string &retsig, const std::string &signature, const FunctorParameters &in, FunctorResult out);

  protected:
    MetaObject *_meta;
  };

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int Object::advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method)
  {
    std::string signature(name);
    std::string sigret;
    signature += "::(";
    typedef typename boost::function_types::parameter_types<METHOD_TYPE>::type MemArgsType;
    typedef typename boost::mpl::pop_front< MemArgsType >::type                ArgsType;

    boost::mpl::for_each< boost::mpl::transform_view<ArgsType, boost::remove_reference<boost::mpl::_1> > >(qi::detail::signature_function_arg_apply(signature));
    signature += ")";
    typedef typename boost::function_types::result_type<METHOD_TYPE>::type     ResultType;
    signatureFromType<ResultType>::value(sigret);
    return xAdvertiseMethod(sigret, signature, makeFunctor(object, method));
  }

  template <typename FUNCTION_TYPE>
  inline unsigned int Object::advertiseMethod(const std::string& name, FUNCTION_TYPE function)
  {
    std::string signature(name);
    std::string sigret;
    signature += "::(";
    typedef typename boost::function_types::parameter_types<FUNCTION_TYPE>::type ArgsType;
    boost::mpl::for_each< boost::mpl::transform_view<ArgsType, boost::remove_reference<boost::mpl::_1> > >(qi::detail::signature_function_arg_apply(signature));
    signature += ")";
    typedef typename boost::function_types::result_type<FUNCTION_TYPE>::type     ResultType;
    signatureFromType<ResultType>::value(sigret);
    return xAdvertiseMethod(sigret, signature, makeFunctor(function));
  }





};

#include <qimessaging/object.hxx>
#endif  // _QIMESSAGING_OBJECT_HPP_

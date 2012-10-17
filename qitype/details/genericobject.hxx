#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICOBJECT_HXX_
#define _QITYPE_DETAILS_GENERICOBJECT_HXX_

#include <qi/future.hpp>
#include <qitype/typeobject.hpp>

namespace qi {

  template<typename T>
  GenericValue makeObjectValue(T* ptr)
  {
    GenericValue res = GenericValue::from(*ptr);
    qiLogDebug("meta") <<"metaobject on " << ptr <<" " << res.value;
    return res;
  }

  // We need to specialize Manager on genericobject to make a copy
  namespace detail {
  template<> struct TypeManager<GenericObject>: public TypeManagerDefault<GenericObject>
  {
    static void* create()
    {
      return new GenericObject(0, 0);
    }
    static void copy(void* dst, const void* src)
    {
      TypeManagerDefault<GenericObject>::copy(dst, src);
      GenericObject* o = (GenericObject*)dst;
      o->value = o->type->clone(o->value);
    }
    static void destroy(void* ptr)
    {
      GenericObject* go = (GenericObject*)ptr;
      go->type->destroy(go->value);
      delete go;
    }
  }; }

  namespace detail
  {

    template <typename T>
    inline void futureAdapter(qi::Future<qi::GenericValue> metaFut, qi::Promise<T> promise)
    {

      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }

      GenericValue val =  metaFut.value();
      Type* targetType = typeOf<T>();
      std::pair<GenericValue, bool> conv = val.convert(targetType);
      if (!conv.first.type)
        promise.setError("Unable to convert call result to target type");
      else
      {
        T* res = (T*)conv.first.type->ptrFromStorage(&conv.first.value);
        promise.setValue(*res);
      }
      if (conv.second)
        conv.first.destroy();
      val.destroy();
    }

    template <>
    inline void futureAdapter<void>(qi::Future<qi::GenericValue> metaFut, qi::Promise<void> promise)
    {
      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }
      promise.setValue(0);
    }

  }




  template<typename R>
  qi::FutureSync<R> GenericObject::call(const std::string& methodName,
    qi::AutoGenericValue p1,
      qi::AutoGenericValue p2,
      qi::AutoGenericValue p3,
      qi::AutoGenericValue p4,
      qi::AutoGenericValue p5,
      qi::AutoGenericValue p6,
      qi::AutoGenericValue p7,
      qi::AutoGenericValue p8)
  {
    qi::Promise<R> res;
    if (!value || !type) {
      res.setError("Invalid GenericObject");
      return res.future();
    }
    qi::AutoGenericValue* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::GenericValue> params;
    for (unsigned i=0; i<8; ++i)
      if (vals[i]->type) // 0 is a perfectly legal value
        params.push_back(*vals[i]);

    // Signature construction
    std::string signature = methodName + "::(";
    for (unsigned i=0; i< params.size(); ++i)
      signature += params[i].signature();
    signature += ")";
    std::string sigret;
    // Go through MetaType::signature which can return unknown, since
    // signatureFroType will produce a static assert
    sigret = typeOf<R>()->signature();
    // Future adaptation
    qi::Future<qi::GenericValue> fmeta = xMetaCall(sigret, signature, params);
    fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));



    return res.future();
  }
}
#endif  // _QITYPE_DETAILS_GENERICOBJECT_HXX_

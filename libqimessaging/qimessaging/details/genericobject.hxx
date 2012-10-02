#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_GENERICOBJECT_HXX_
#define _QIMESSAGING_DETAILS_GENERICOBJECT_HXX_

#include <qimessaging/buffer.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/metafunction.hpp>

namespace qi {

  template<typename T>
  GenericValue makeObjectValue(T* ptr)
  {
    GenericValue res = toValue(*ptr);
    qiLogDebug("meta") <<"metaobject on " << ptr <<" " << res.value;
    return res;
  }

  // We need to specialize Type on genericobject to make a copy
  template<> class TypeDefaultClone<TypeDefaultAccess<GenericObject> >
  {
  public:
    static void* clone(void* src)
    {
      GenericObject* osrc = (GenericObject*)src;
      GenericObject* res = new GenericObject(*osrc);
      res->value = res->type->clone(res->value);
      return res;
     }
    static void destroy(void* ptr)
    {
      GenericObject* go = (GenericObject*)ptr;
      go->type->destroy(go->value);
      delete (GenericObject*)ptr;
    }
  };

  namespace detail
  {

    template <typename T>
    inline void futureAdapter(qi::Future<qi::MetaFunctionResult> metaFut, qi::Promise<T> promise) {

      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }

      //put metaFutureResult in promise<T>
      if (metaFut.value().getMode() == MetaFunctionResult::Mode_GenericValue)
      {
        GenericValue val =  metaFut.value().getValue();
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
      }
      else
      {
        IDataStream in(metaFut.value().getBuffer());
        // Not all types are serializable, go through MetaType
        Type* type = typeOf<T>();
        void* storage = type->deserialize(in);
        if (!storage)
        {
          promise.setError("Could not deserialize result");
        }
        else
        {
          void* ptr = type->ptrFromStorage(&storage);
          promise.setValue(*(T*)ptr);
          type->destroy(storage);
        }
      }
    }

    template <>
    inline void futureAdapter<void>(qi::Future<qi::MetaFunctionResult> metaFut, qi::Promise<void> promise) {

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
    // Mark params as being on the stack
    MetaFunctionParameters p(params, true);
    qi::Future<qi::MetaFunctionResult> fmeta = xMetaCall(sigret, signature, p);
    fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));



    return res.future();
  }
}
#endif  // _QIMESSAGING_DETAILS_GENERICOBJECT_HXX_

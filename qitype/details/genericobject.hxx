#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICOBJECT_HXX_
#define _QITYPE_DETAILS_GENERICOBJECT_HXX_

#include <qi/future.hpp>
#include <qitype/typeobject.hpp>
#include <qitype/details/typeimpl.hxx>

namespace qi {

  namespace detail
  {

    template <typename T>
    inline void futureAdapter(qi::Future<qi::GenericValuePtr> metaFut, qi::Promise<T> promise)
    {

      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }

      GenericValuePtr val =  metaFut.value();
      Type* targetType = typeOf<T>();
      std::pair<GenericValuePtr, bool> conv = val.convert(targetType);
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
    inline void futureAdapter<void>(qi::Future<qi::GenericValuePtr> metaFut, qi::Promise<void> promise)
    {
      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }
      promise.setValue(0);
    }

  }

  inline EventLoop* GenericObject::eventLoop() const
  {
    // Use the Manageable in the value, or fallback to the one in GenericObject
    Manageable* m = type->manageable(value);
    return m?m->eventLoop():Manageable::eventLoop();
  }

  inline void GenericObject::moveToEventLoop(EventLoop* eventLoop)
  {
    Manageable* m = type->manageable(value);
    if (m)
      m->moveToEventLoop(eventLoop);
    else
      Manageable::moveToEventLoop(eventLoop);
  }

  template<typename R>
  qi::FutureSync<R> GenericObject::call(const std::string& methodName,
    qi::AutoGenericValuePtr p1,
      qi::AutoGenericValuePtr p2,
      qi::AutoGenericValuePtr p3,
      qi::AutoGenericValuePtr p4,
      qi::AutoGenericValuePtr p5,
      qi::AutoGenericValuePtr p6,
      qi::AutoGenericValuePtr p7,
      qi::AutoGenericValuePtr p8)
  {
    qi::Promise<R> res;
    if (!value || !type) {
      res.setError("Invalid GenericObject");
      return res.future();
    }
    qi::AutoGenericValuePtr* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::GenericValuePtr> params;
    for (unsigned i=0; i<8; ++i)
      if (vals[i]->type) // 0 is a perfectly legal value
        params.push_back(*vals[i]);

    // Signature construction
    std::string signature = methodName + "::(";
    for (unsigned i=0; i< params.size(); ++i)
      signature += params[i].signature();
    signature += ")";
    std::string sigret;

    // Future adaptation
    qi::Future<qi::GenericValuePtr> fmeta = metaCall(signature, params);
    fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));

    return res.future();
  }

  /* An ObjectPtr is actually of a Dynamic type: The underlying Type*
   * is not allways the same.
  */
  template<> class TypeImpl<ObjectPtr>: public TypeDynamic
  {
  public:
    virtual std::pair<GenericValuePtr, bool> get(void* storage)
    {
      ObjectPtr* val = (ObjectPtr*)ptrFromStorage(&storage);
      GenericValuePtr result;
      result.type = (*val)->type;
      result.value = (*val)->value;
      return std::make_pair(result, false);
    }

    virtual void set(void** storage, GenericValuePtr source)
    {
      ObjectPtr* val = (ObjectPtr*)ptrFromStorage(storage);
      if (source.kind() != Type::Object)
        throw std::runtime_error("Cannot assign non-object to ObjectPtr");
      ObjectPtr op(new GenericObject(static_cast<ObjectType*>(source.type), source.value));
      *val = op;
    }
    typedef DefaultTypeImplMethods<ObjectPtr> Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

}
#endif  // _QITYPE_DETAILS_GENERICOBJECT_HXX_

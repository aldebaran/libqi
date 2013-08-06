#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICOBJECT_HXX_
#define _QITYPE_DETAILS_GENERICOBJECT_HXX_

#include <qi/future.hpp>
#include <qitype/typeinterface.hpp>
#include <qitype/typeobject.hpp>
#include <qitype/details/typeimpl.hxx>
#include <qitype/objecttypebuilder.hpp>

QI_REGISTER_TEMPLATE_OBJECT(qi::Future    , _connect, isFinished, value, wait, isRunning, isCanceled, hasError, error);
QI_REGISTER_TEMPLATE_OBJECT(qi::FutureSync, _connect, isFinished, value, wait, isRunning, isCanceled, hasError, error, async);

namespace qi {

  namespace detail
  {

    template<typename T> void hold(T data) {}

    template <typename T>
    void futureAdapterGeneric(AnyReference val, qi::Promise<T> promise)
    {
      TemplateTypeInterface* ft1 = QI_TEMPLATE_TYPE_GET(val.type, Future);
      TemplateTypeInterface* ft2 = QI_TEMPLATE_TYPE_GET(val.type, FutureSync);
      TemplateTypeInterface* futureType = ft1 ? ft1 : ft2;
      ObjectTypeInterface* onext = dynamic_cast<ObjectTypeInterface*>(futureType->next());
      GenericObject gfut(onext, val.value);
      // Need a live shared_ptr for shared_from_this() to work.
      AnyObject ao(&gfut, hold<GenericObject*>);
      if (gfut.call<bool>(MetaCallType_Direct, "hasError", 0))
      {
        promise.setError(gfut.call<std::string>("error", 0));
        return;
      }
      AnyValue v = gfut.call<AnyValue>(MetaCallType_Direct, "value", 0);
      promise.setValue(v.to<T>());
      val.destroy();
    }

    template <typename T>
    inline void futureAdapter(qi::Future<qi::AnyReference> metaFut, qi::Promise<T> promise)
    {
      qiLogDebug("qi.object") << "futureAdapter";
      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }

      AnyReference val =  metaFut.value();
      TemplateTypeInterface* ft1 = QI_TEMPLATE_TYPE_GET(val.type, Future);
      TemplateTypeInterface* ft2 = QI_TEMPLATE_TYPE_GET(val.type, FutureSync);
      TemplateTypeInterface* futureType = ft1 ? ft1 : ft2;
      qiLogDebug("qi.object") << "isFuture " << !!ft1 << ' ' << !!ft2;
      if (futureType)
      {
        TypeInterface* next = futureType->next();
        ObjectTypeInterface* onext = dynamic_cast<ObjectTypeInterface*>(next);
        GenericObject gfut(onext, val.value);
        // Need a live shared_ptr for shared_from_this() to work.
        AnyObject ao(&gfut, &hold<GenericObject*>);
        boost::function<void()> cb = boost::bind(futureAdapterGeneric<T>, val, promise);
        // Careful, gfut will die at the end of this block, but it is
        // stored in call data. So call must finish before we exit this block,
        // and thus must be synchronous.
        qi::Future<void> waitResult = gfut.call<void>(MetaCallType_Direct, "_connect", cb);
        waitResult.wait();
        qiLogDebug("qi.object") << "future connected " << !waitResult.hasError();
        if (waitResult.hasError())
          qiLogWarning("qi.object") << waitResult.error();
        return;
      }
      TypeInterface* targetType = typeOf<T>();
      try
      {
        std::pair<AnyReference, bool> conv = val.convert(targetType);
        if (!conv.first.type)
          promise.setError(std::string("Unable to convert call result to target type:")
            + val.type->infoString() + " -> " + targetType->infoString());
        else
        {
          T* res = (T*)conv.first.type->ptrFromStorage(&conv.first.value);
          promise.setValue(*res);
        }
        if (conv.second)
          conv.first.destroy();
      }
      catch(const std::exception& e)
      {
        promise.setError(std::string("Return argument conversion error: ") + e.what());
      }
      val.destroy();
    }

    template <typename T>
    inline void futureAdapterVal(qi::Future<qi::AnyValue> metaFut, qi::Promise<T> promise)
    {
      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }
      const AnyValue& val =  metaFut.value();
      try
      {
        promise.setValue(val.to<T>());
      }
      catch (const std::exception& e)
      {
        promise.setError(std::string("Return argument conversion error: ") + e.what());
      }
    }

    template<>
    inline void futureAdapterVal(qi::Future<qi::AnyValue> metaFut, qi::Promise<AnyValue> promise)
    {
      if (metaFut.hasError())
        promise.setError(metaFut.error());
      else
        promise.setValue(metaFut.value());
    }

    template <>
    inline void futureAdapter<void>(qi::Future<qi::AnyReference> metaFut, qi::Promise<void> promise)
    {
      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }
      promise.setValue(0);
    }

  }


  /* Generate qi::FutureSync<R> GenericObject::call(methodname, args...)
   * for all argument counts
   * The function packs arguments in a vector<AnyReference>, computes the
   * signature and bounce those to metaCall.
   */
  #define pushi(z, n,_) params.push_back(p ## n);
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                           \
  template<typename R> qi::FutureSync<R> GenericObject::call(             \
      const std::string& methodName       comma                           \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))             \
  {                                                                        \
     if (!value || !type) {                                                \
      return makeFutureError<R>("Invalid GenericObject");                  \
     }                                                                     \
     std::vector<qi::AnyReference> params;                              \
     params.reserve(n);                                                    \
     BOOST_PP_REPEAT(n, pushi, _)                                          \
     std::string sigret;                                                   \
     qi::Promise<R> res(qi::FutureCallbackType_Sync);                      \
     qi::Future<AnyReference> fmeta = metaCall(methodName, params);      \
     fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));  \
     return res.future();                                                  \
  }

  QI_GEN(genCall)
  #undef genCall
  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                           \
  template<typename R> qi::FutureSync<R> GenericObject::async(             \
      const std::string& methodName       comma                           \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))             \
  {                                                                        \
     if (!value || !type) {                                                \
      return makeFutureError<R>("Invalid GenericObject");                  \
     }                                                                     \
     std::vector<qi::AnyReference> params;                              \
     params.reserve(n);                                                    \
     BOOST_PP_REPEAT(n, pushi, _)                                          \
     std::string sigret;                                                   \
     qi::Promise<R> res(qi::FutureCallbackType_Sync);                                            \
     qi::Future<AnyReference> fmeta = metaCall(methodName, params, MetaCallType_Queued);   \
     fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));  \
     return res.future();                                                  \
  }

  QI_GEN(genCall)
  #undef genCall
  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)               \
  template<typename R> qi::FutureSync<R> GenericObject::call(             \
      MetaCallType callType,                                              \
      const std::string& methodName       comma                           \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))             \
  {                                                                        \
     if (!value || !type) {                                                \
      return makeFutureError<R>("Invalid GenericObject");                  \
     }                                                                     \
     std::vector<qi::AnyReference> params;                              \
     params.reserve(n);                                                    \
     BOOST_PP_REPEAT(n, pushi, _)                                          \
     std::string sigret;                                                   \
     qi::Promise<R> res(qi::FutureCallbackType_Sync);                       \
     qi::Future<AnyReference> fmeta = metaCall(methodName, params, callType);   \
     fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));  \
     return res.future();                                                  \
  }

  QI_GEN(genCall)
  #undef genCall

  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)         \
  template<typename R,typename T> qi::FutureSync<R> async(   \
      T* instance,                                                 \
      const std::string& methodName comma                         \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference)) \
  {                                                              \
    AnyObject obj = AnyReference(instance).toObject();       \
    qi::Future<R> res = obj->call<R>(MetaCallType_Queued, methodName comma AUSE);  \
    res.connect(boost::bind(&detail::hold<AnyObject>, obj));   \
    return res;                                                 \
  }
  QI_GEN(genCall)
  #undef genCall

  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)         \
  template<typename R,typename T> qi::FutureSync<R> async(   \
      boost::shared_ptr<T> instance,                             \
      const std::string& methodName comma                         \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference)) \
  {                                                              \
    AnyObject obj = AnyReference(instance).toObject();        \
    qi::Future<R> res =  obj->call<R>(MetaCallType_Queued, methodName comma AUSE);  \
    res.connect(boost::bind(&detail::hold<AnyObject>, obj));   \
    return res;                                                 \
  }
  QI_GEN(genCall)
  #undef genCall
  #undef pushi

  template<typename T>
  qi::FutureSync<T> GenericObject::property(const std::string& name)
  {
    int pid = metaObject().propertyId(name);
    if (pid < 0)
      return makeFutureError<T>("Property not found");
    qi::Future<AnyValue> f = type->property(value, pid);
    qi::Promise<T> p(qi::FutureCallbackType_Sync);
    f.connect(boost::bind(&detail::futureAdapterVal<T>,_1, p));
    return p.future();
  }

  template<typename T>
  qi::FutureSync<void> GenericObject::setProperty(const std::string& name, const T& val)
  {
    int pid = metaObject().propertyId(name);
    if (pid < 0)
      return makeFutureError<void>("Property not found");
    return type->setProperty(value, pid, AnyValue(AnyReference(val)));
  }

  /* An AnyObject is actually of a Dynamic type: The underlying TypeInterface*
   * is not allways the same.
  */
  template<> class QITYPE_API TypeImpl<AnyObject>: public DynamicTypeInterface
  {
  public:
    virtual AnyReference get(void* storage)
    {
      AnyObject* val = (AnyObject*)ptrFromStorage(&storage);
      AnyReference result;
      if (!*val)
      {
        return AnyReference();
      }
      result.type = (*val)->type;
      result.value = (*val)->value;
      return result;
    }

    virtual void set(void** storage, AnyReference source)
    {
      qiLogCategory("qitype.object");
      AnyObject* val = (AnyObject*)ptrFromStorage(storage);
      TemplateTypeInterface* templ = dynamic_cast<TemplateTypeInterface*>(source.type);
      if (templ)
        source.type = templ->next();
      if (source.type->info() == info())
      { // source is objectptr
        AnyObject* src = (AnyObject*)source.type->ptrFromStorage(&source.value);
        if (!*src)
          qiLogWarning() << "NULL AnyObject";
        *val = *src;
      }
      else if (source.kind() == TypeKind_Dynamic)
      { // try to dereference dynamic type in case it contains an object
        set(storage, source.asDynamic());
      }
      else if (source.kind() == TypeKind_Object)
      { // wrap object in objectptr: we do not keep it alive,
        // but source type offers no tracking capability
        AnyObject op(new GenericObject(static_cast<ObjectTypeInterface*>(source.type), source.value));
        *val = op;
      }
      else if (source.kind() == TypeKind_Pointer)
      {
        PointerTypeInterface* ptype = static_cast<PointerTypeInterface*>(source.type);
        // FIXME: find a way!
        if (ptype->pointerKind() == PointerTypeInterface::Shared)
          qiLogInfo() << "AnyObject will *not* track original shared pointer";
        set(storage, *source);
      }
      else
        throw std::runtime_error((std::string)"Cannot assign non-object " + source.type->infoString() + " to AnyObject");

    }
    typedef DefaultTypeImplMethods<AnyObject> Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  namespace detail
  {
    typedef std::map<TypeInfo, boost::function<AnyReference(AnyObject)> > ProxyGeneratorMap;
    QITYPE_API ProxyGeneratorMap& proxyGeneratorMap();

    template<typename Proxy>
    AnyReference makeProxy(AnyObject ptr)
    {
      boost::shared_ptr<Proxy> sp(new Proxy(ptr));
      return AnyReference(sp).clone();
    }
  }
  template<typename Proxy, typename Interface>
  bool registerProxyInterface()
  {
    detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
    map[typeOf<Interface>()->info()] = &detail::makeProxy<Proxy>;
    return true;
  }
  template<typename Proxy>
  bool registerProxy()
  {
    detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
    map[typeOf<Proxy>()->info()] = &detail::makeProxy<Proxy>;
    return true;
  }

  namespace detail
  {
    /* Factory helper functions
    */

    // create a T, wrap in a AnyObject
    template<typename T> AnyObject constructObject()
    {
      return AnyReference::fromPtr(new T()).toObject();
    }

    // in genericobjectbuilder.hxx
    template<typename T> AnyObject makeObject(const std::string& fname, T func);

    // Create a factory function for an object with one method functionName bouncing to func
    template<typename T> boost::function<AnyObject(const std::string&)>
    makeObjectFactory(const std::string functionName, T func)
    {
      return ( boost::function<AnyObject(const std::string&)>)
        boost::bind(&makeObject<T>, functionName, func);
    }
  }
}
QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::MinMaxSum,
  ("minValue",       minValue),
  ("maxValue",       maxValue),
  ("cumulatedValue", cumulatedValue));
QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::MethodStatistics,
  ("count",  count),
  ("wall",   wall),
  ("user",   user),
  ("system", system));
QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::EventTrace,
  ("id",           id),
  ("kind",         kind),
  ("slotId",       slotId),
  ("arguments",    arguments),
  ("timestamp",    timestamp),
  ("userUsTime",   userUsTime),
  ("systemUsTime", systemUsTime));
QI_TYPE_STRUCT(qi::os::timeval, tv_sec, tv_usec);
#endif  // _QITYPE_DETAILS_GENERICOBJECT_HXX_

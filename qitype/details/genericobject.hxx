#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICOBJECT_HXX_
#define _QITYPE_DETAILS_GENERICOBJECT_HXX_

#include <qi/future.hpp>
#include <qitype/type.hpp>
#include <qitype/typeobject.hpp>
#include <qitype/details/typeimpl.hxx>
#include <qitype/objecttypebuilder.hpp>

QI_REGISTER_TEMPLATE_OBJECT(qi::Future    , connect, _connect, isFinished, value, wait, isRunning, isCanceled, hasError, error);
QI_REGISTER_TEMPLATE_OBJECT(qi::FutureSync, connect, _connect, isFinished, value, wait, isRunning, isCanceled, hasError, error, async);

namespace qi {

  namespace detail
  {

    template<typename T> void hold(T data) {}

    template <typename T>
    void futureAdapterGeneric(GenericValuePtr val, qi::Promise<T> promise)
    {
      TemplateTypeInterface* ft1 = QI_TEMPLATE_TYPE_GET(val.type, Future);
      TemplateTypeInterface* ft2 = QI_TEMPLATE_TYPE_GET(val.type, FutureSync);
      TemplateTypeInterface* futureType = ft1 ? ft1 : ft2;
      ObjectTypeInterface* onext = dynamic_cast<ObjectTypeInterface*>(futureType->next());
      GenericObject gfut(onext, val.value);
      if (gfut.call<bool>("hasError", 0))
      {
        promise.setError(gfut.call<std::string>("error", 0));
        return;
      }
      GenericValue v = gfut.call<GenericValue>("value", 0);
      promise.setValue(v.to<T>());
      val.destroy();
    }

    template <typename T>
    inline void futureAdapter(qi::Future<qi::GenericValuePtr> metaFut, qi::Promise<T> promise)
    {

      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }

      GenericValuePtr val =  metaFut.value();
      TemplateTypeInterface* ft1 = QI_TEMPLATE_TYPE_GET(val.type, Future);
      TemplateTypeInterface* ft2 = QI_TEMPLATE_TYPE_GET(val.type, FutureSync);
      TemplateTypeInterface* futureType = ft1 ? ft1 : ft2;
      if (futureType)
      {
        TypeInterface* next = futureType->next();
        ObjectTypeInterface* onext = dynamic_cast<ObjectTypeInterface*>(next);
        GenericObject gfut(onext, val.value);
        boost::function<void()> cb = boost::bind(futureAdapterGeneric<T>, val, promise);
        gfut.call<void>("_connect", cb);
        return;
      }
      TypeInterface* targetType = typeOf<T>();
      try
      {
        std::pair<GenericValuePtr, bool> conv = val.convert(targetType);
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
    inline void futureAdapterVal(qi::Future<qi::GenericValue> metaFut, qi::Promise<T> promise)
    {
      //error handling
      if (metaFut.hasError()) {
        promise.setError(metaFut.error());
        return;
      }
      const GenericValue& val =  metaFut.value();
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
    inline void futureAdapterVal(qi::Future<qi::GenericValue> metaFut, qi::Promise<GenericValue> promise)
    {
      if (metaFut.hasError())
        promise.setError(metaFut.error());
      else
        promise.setValue(metaFut.value());
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


  /* Generate qi::FutureSync<R> GenericObject::call(methodname, args...)
   * for all argument counts
   * The function packs arguments in a vector<GenericValuePtr>, computes the
   * signature and bounce those to metaCall.
   */
  #define pushi(z, n,_) params.push_back(p ## n);
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                           \
  template<typename R> qi::FutureSync<R> GenericObject::call(             \
      const std::string& methodName       comma                           \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoGenericValuePtr))             \
  {                                                                        \
     if (!value || !type) {                                                \
      return makeFutureError<R>("Invalid GenericObject");                  \
     }                                                                     \
     std::vector<qi::GenericValuePtr> params;                              \
     params.reserve(n);                                                    \
     BOOST_PP_REPEAT(n, pushi, _)                                          \
     std::string sigret;                                                   \
     qi::Promise<R> res;                                                   \
     qi::Future<GenericValuePtr> fmeta = metaCall(methodName, params);      \
     fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));  \
     return res.future();                                                  \
  }

  QI_GEN(genCall)
  #undef genCall
  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                           \
  template<typename R> qi::FutureSync<R> GenericObject::async(             \
      const std::string& methodName       comma                           \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoGenericValuePtr))             \
  {                                                                        \
     if (!value || !type) {                                                \
      return makeFutureError<R>("Invalid GenericObject");                  \
     }                                                                     \
     std::vector<qi::GenericValuePtr> params;                              \
     params.reserve(n);                                                    \
     BOOST_PP_REPEAT(n, pushi, _)                                          \
     std::string sigret;                                                   \
     qi::Promise<R> res;                                                   \
     qi::Future<GenericValuePtr> fmeta = metaCall(methodName, params, MetaCallType_Queued);   \
     fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));  \
     return res.future();                                                  \
  }

  QI_GEN(genCall)
  #undef genCall
  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)               \
  template<typename R> qi::FutureSync<R> GenericObject::call(             \
      MetaCallType callType,                                              \
      const std::string& methodName       comma                           \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoGenericValuePtr))             \
  {                                                                        \
     if (!value || !type) {                                                \
      return makeFutureError<R>("Invalid GenericObject");                  \
     }                                                                     \
     std::vector<qi::GenericValuePtr> params;                              \
     params.reserve(n);                                                    \
     BOOST_PP_REPEAT(n, pushi, _)                                          \
     std::string sigret;                                                   \
     qi::Promise<R> res;                                                   \
     qi::Future<GenericValuePtr> fmeta = metaCall(methodName, params, callType);   \
     fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));  \
     return res.future();                                                  \
  }

  QI_GEN(genCall)
  #undef genCall

  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)         \
  template<typename R,typename T> qi::FutureSync<R> async(   \
      T* instance,                                                 \
      const std::string& methodName comma                         \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoGenericValuePtr)) \
  {                                                              \
    ObjectPtr obj = GenericValueRef(instance).toObject();       \
    qi::Future<R> res = obj->call<R>(MetaCallType_Queued, methodName comma AUSE);  \
    res.connect(boost::bind(&detail::hold<ObjectPtr>, obj));   \
    return res;                                                 \
  }
  QI_GEN(genCall)
  #undef genCall

  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)         \
  template<typename R,typename T> qi::FutureSync<R> async(   \
      boost::shared_ptr<T> instance,                             \
      const std::string& methodName comma                         \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoGenericValuePtr)) \
  {                                                              \
    ObjectPtr obj = GenericValueRef(instance).toObject();        \
    qi::Future<R> res =  obj->call<R>(MetaCallType_Queued, methodName comma AUSE);  \
    res.connect(boost::bind(&detail::hold<ObjectPtr>, obj));   \
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
    qi::Future<GenericValue> f = type->property(value, pid);
    qi::Promise<T> p;
    f.connect(boost::bind(&detail::futureAdapterVal<T>,_1, p));
    return p.future();
  }

  template<typename T>
  qi::FutureSync<void> GenericObject::setProperty(const std::string& name, const T& val)
  {
    int pid = metaObject().propertyId(name);
    if (pid < 0)
      return makeFutureError<void>("Property not found");
    return type->setProperty(value, pid, GenericValue(GenericValueRef(val)));
  }

  /* An ObjectPtr is actually of a Dynamic type: The underlying TypeInterface*
   * is not allways the same.
  */
  template<> class QITYPE_API TypeImpl<ObjectPtr>: public DynamicTypeInterface
  {
  public:
    virtual GenericValuePtr get(void* storage)
    {
      ObjectPtr* val = (ObjectPtr*)ptrFromStorage(&storage);
      GenericValuePtr result;
      if (!*val)
      {
        return GenericValuePtr();
      }
      result.type = (*val)->type;
      result.value = (*val)->value;
      return result;
    }

    virtual void set(void** storage, GenericValuePtr source)
    {
      qiLogCategory("qitype.object");
      ObjectPtr* val = (ObjectPtr*)ptrFromStorage(storage);
      TemplateTypeInterface* templ = dynamic_cast<TemplateTypeInterface*>(source.type);
      if (templ)
        source.type = templ->next();
      if (source.type->info() == info())
      { // source is objectptr
        ObjectPtr* src = (ObjectPtr*)source.type->ptrFromStorage(&source.value);
        if (!*src)
          qiLogWarning() << "NULL ObjectPtr";
        *val = *src;
      }
      else if (source.kind() == TypeInterface::Dynamic)
      { // try to dereference dynamic type in case it contains an object
        set(storage, source.asDynamic());
      }
      else if (source.kind() == TypeInterface::Object)
      { // wrap object in objectptr: we do not keep it alive,
        // but source type offers no tracking capability
        ObjectPtr op(new GenericObject(static_cast<ObjectTypeInterface*>(source.type), source.value));
        *val = op;
      }
      else if (source.kind() == TypeInterface::Pointer)
      {
        PointerTypeInterface* ptype = static_cast<PointerTypeInterface*>(source.type);
        // FIXME: find a way!
        if (ptype->pointerKind() == PointerTypeInterface::Shared)
          qiLogInfo() << "ObjectPtr will *not* track original shared pointer";
        set(storage, *source);
      }
      else
        throw std::runtime_error((std::string)"Cannot assign non-object " + source.type->infoString() + " to ObjectPtr");

    }
    typedef DefaultTypeImplMethods<ObjectPtr> Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  namespace detail
  {
    typedef std::map<TypeInfo, boost::function<GenericValuePtr(ObjectPtr)> > ProxyGeneratorMap;
    QITYPE_API ProxyGeneratorMap& proxyGeneratorMap();

    template<typename Proxy>
    GenericValuePtr makeProxy(ObjectPtr ptr)
    {
      boost::shared_ptr<Proxy> sp(new Proxy(ptr));
      return GenericValuePtr::fromRef(sp).clone();
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

    // create a T, wrap in a ObjectPtr
    template<typename T> ObjectPtr constructObject()
    {
      return GenericValuePtr::fromPtr(new T()).toObject();
    }

    // in genericobjectbuilder.hxx
    template<typename T> ObjectPtr makeObject(const std::string& fname, T func);

    // Create a factory function for an object with one method functionName bouncing to func
    template<typename T> boost::function<ObjectPtr(const std::string&)>
    makeObjectFactory(const std::string functionName, T func)
    {
      return ( boost::function<ObjectPtr(const std::string&)>)
        boost::bind(&makeObject<T>, functionName, func);
    }
  }
}
QI_TYPE_STRUCT(qi::MethodStatistics, cumulatedTime, minTime, maxTime, count);
QI_TYPE_STRUCT(qi::EventTrace, id, kind, slotId, arguments, timestamp);
QI_TYPE_STRUCT(qi::os::timeval, tv_sec, tv_usec);
#endif  // _QITYPE_DETAILS_GENERICOBJECT_HXX_

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
     std::string signature = methodName + "::(";                           \
     for (unsigned i=0; i< params.size(); ++i)                             \
       signature += params[i].signature();                                 \
     signature += ")";                                                     \
     std::string sigret;                                                   \
     qi::Promise<R> res;                                                   \
     qi::Future<GenericValuePtr> fmeta = metaCall(signature, params);      \
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
     std::string signature = methodName + "::(";                           \
     for (unsigned i=0; i< params.size(); ++i)                             \
       signature += params[i].signature();                                 \
     signature += ")";                                                     \
     std::string sigret;                                                   \
     qi::Promise<R> res;                                                   \
     qi::Future<GenericValuePtr> fmeta = metaCall(signature, params, MetaCallType_Queued);   \
     fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));  \
     return res.future();                                                  \
  }

  QI_GEN(genCall)
  #undef genCall
  #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                           \
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
     std::string signature = methodName + "::(";                           \
     for (unsigned i=0; i< params.size(); ++i)                             \
       signature += params[i].signature();                                 \
     signature += ")";                                                     \
     std::string sigret;                                                   \
     qi::Promise<R> res;                                                   \
     qi::Future<GenericValuePtr> fmeta = metaCall(signature, params, callType);   \
     fmeta.connect(boost::bind<void>(&detail::futureAdapter<R>, _1, res));  \
     return res.future();                                                  \
  }

  QI_GEN(genCall)
  #undef genCall
  #undef pushi

  template<typename T>
  qi::FutureSync<T> GenericObject::getProperty(const std::string& name)
  {
    int pid = metaObject().propertyId(name);
    if (pid < 0)
      return makeFutureError<T>("Property not found");
    qi::Future<GenericValue> f = type->getProperty(value, pid);
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

  /* An ObjectPtr is actually of a Dynamic type: The underlying Type*
   * is not allways the same.
  */
  template<> class QITYPE_API TypeImpl<ObjectPtr>: public TypeDynamic
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
      if (source.type->info() == info())
      { // source is objectptr
        ObjectPtr* src = (ObjectPtr*)source.type->ptrFromStorage(&source.value);
        if (!*src)
          qiLogWarning() << "NULL ObjectPtr";
        *val = *src;
      }
      else if (source.kind() == Type::Dynamic)
      { // try to dereference dynamic type in case it contains an object
        set(storage, source.asDynamic());
      }
      else if (source.kind() == Type::Object)
      { // wrap object in objectptr: we do not keep it alive,
        // but source type offers no tracking capability
        ObjectPtr op(new GenericObject(static_cast<ObjectType*>(source.type), source.value));
        *val = op;
      }
      else if (source.kind() == Type::Pointer)
      {
        TypePointer* ptype = static_cast<TypePointer*>(source.type);
        // FIXME: find a way!
        if (ptype->pointerKind() == TypePointer::Shared)
          qiLogInfo("ObjectPtr will *not* track original shared pointer");
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
      return GenericValuePtr(&sp).clone();
    }
  }
  template<typename Proxy>
  bool registerProxy()
  {
    detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
    map[typeOf<Proxy>()->info()] = &detail::makeProxy<Proxy>;
    return true;
  }
}
#endif  // _QITYPE_DETAILS_GENERICOBJECT_HXX_

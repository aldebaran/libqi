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
  #undef pushi

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

#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_DETAILS_ASYNC_HXX_
#define _QI_TYPE_DETAILS_ASYNC_HXX_

#include <boost/mpl/if.hpp>

#include <qi/future.hpp>

namespace qi {

#ifdef DOXYGEN
  /** Perform an asynchronous call on a method.
   * The argument may be a pointer or shared-pointer to an instance of a class
   * known to type system.
   */
  template<typename R, typename T>
  qi::Future<R> async(T instancePointerOrSharedPointer,
                      const std::string& methodName,
                      qi::AutoAnyReference p1 = qi::AutoAnyReference(),
                      qi::AutoAnyReference p2 = qi::AutoAnyReference(),
                      qi::AutoAnyReference p3 = qi::AutoAnyReference(),
                      qi::AutoAnyReference p4 = qi::AutoAnyReference(),
                      qi::AutoAnyReference p5 = qi::AutoAnyReference(),
                      qi::AutoAnyReference p6 = qi::AutoAnyReference(),
                      qi::AutoAnyReference p7 = qi::AutoAnyReference(),
                      qi::AutoAnyReference p8 = qi::AutoAnyReference());
#else
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)    \
  template<typename R,typename T> qi::Future<R> async(       \
      T* instance,                                           \
      const std::string& methodName comma                    \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))      \
  {                                                          \
    AnyObject obj = AnyReference::from(instance).toObject(); \
    qi::Future<R> res = obj.async<R>(                        \
        methodName comma AUSE);                              \
    res.connect(boost::bind(&detail::hold<AnyObject>, obj)); \
    return res;                                              \
  }
QI_GEN(genCall)
#undef genCall

#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)    \
  template<typename R,typename T> qi::Future<R> async(       \
      boost::shared_ptr<T> instance,                         \
      const std::string& methodName comma                    \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))      \
  {                                                          \
    AnyObject obj = AnyReference::from(instance).toObject(); \
    qi::Future<R> res = obj.async<R>(                        \
        methodName comma AUSE);                              \
    res.connect(boost::bind(&detail::hold<AnyObject>, obj)); \
    return res;                                              \
  }
QI_GEN(genCall)
#undef genCall

#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)    \
  template<typename R,typename T> qi::Future<R> async(       \
      Object<T> instance,                                    \
      const std::string& methodName comma                    \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference))      \
  {                                                          \
    AnyObject obj = instance;                                \
    qi::Future<R> res = obj.async<R>(                        \
        methodName comma AUSE);                              \
    res.connect(boost::bind(&detail::hold<AnyObject>, obj)); \
    return res;                                              \
  }
QI_GEN(genCall)
#undef genCall
#endif

}

#endif  // _QITYPE_DETAILS_ASYNC_HXX_

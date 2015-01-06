#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QI_TYPE_DETAIL_FUTURE_ADAPTER_HXX_
#define QI_TYPE_DETAIL_FUTURE_ADAPTER_HXX_

#include <qi/type/detail/futureadapter.hpp>

namespace qi
{
namespace detail
{

template<typename T> void setPromise(qi::Promise<T>& promise, AnyValue& v)
{
  try
  {
    qiLogDebug("qi.adapter") << "converting value";
    T val = v.to<T>();
    qiLogDebug("qi.adapter") << "setting promise";
    promise.setValue(val);
    qiLogDebug("qi.adapter") << "done";
  }
  catch(const std::exception& e)
  {
    qiLogError("qi.adapter") << "future to promise forwarding error: " << e.what();
    promise.setError(e.what());
  }
}

template<> inline void setPromise(qi::Promise<void>& promise, AnyValue&)
{
  promise.setValue(0);
}

template <typename T>
void futureAdapterGeneric(AnyReference val, qi::Promise<T> promise)
{
  qiLogDebug("qi.adapter") << "futureAdapter trigger";
  TypeOfTemplate<Future>* ft1 = QI_TEMPLATE_TYPE_GET(val.type(), Future);
  TypeOfTemplate<FutureSync>* ft2 = QI_TEMPLATE_TYPE_GET(val.type(), FutureSync);
  ObjectTypeInterface* onext = NULL;
  qiLogDebug("qi.object") << "isFuture " << val.type()->infoString() << ' ' << !!ft1 << ' ' << !!ft2;
  bool isvoid = false;
  if (ft1)
  {
    onext = ft1;
    isvoid = ft1->templateArgument()->kind() == TypeKind_Void;
  }
  else if (ft2)
  {
    onext = ft2;
    isvoid = ft2->templateArgument()->kind() == TypeKind_Void;
  }
  assert(onext);
  GenericObject gfut(onext, val.rawValue());
  // Need a live shared_ptr for shared_from_this() to work.
  boost::shared_ptr<GenericObject> ao(&gfut, hold<GenericObject*>);
  if (gfut.call<bool>("hasError", 0))
  {
    qiLogDebug("qi.adapter") << "futureAdapter: future in error";
    std::string s = gfut.call<std::string>("error", 0);
    qiLogDebug("qi.adapter") << "futureAdapter: got error: " << s;
    promise.setError(s);
    return;
  }
  if (gfut.call<bool>("isCanceled"))
  {
    qiLogDebug("qi.adapter") << "futureAdapter: future cancelled";
    promise.setCanceled();
    return;
  }
  qiLogDebug("qi.adapter") << "futureAdapter: future has value";
  AnyValue v = gfut.call<AnyValue>("value", 0);
  // For a Future<void>, value() gave us a void*
  if (isvoid)
    v = AnyValue(qi::typeOf<void>());
  qiLogDebug("qi.adapter") << v.type()->infoString();
  setPromise(promise, v);
  qiLogDebug("qi.adapter") << "Promise set";
  val.destroy();
}

// futureAdapter helper that detects and handles value of kind future
// return true if value was a future and was handled
template <typename T>
inline bool handleFuture(AnyReference val, Promise<T> promise)
{
  TypeOfTemplate<Future>* ft1 = QI_TEMPLATE_TYPE_GET(val.type(), Future);
  TypeOfTemplate<FutureSync>* ft2 = QI_TEMPLATE_TYPE_GET(val.type(), FutureSync);
  ObjectTypeInterface* onext = NULL;
  qiLogDebug("qi.object") << "isFuture " << val.type()->infoString() << ' ' << !!ft1 << ' ' << !!ft2;
  if (ft1)
    onext = ft1;
  else if (ft2)
    onext = ft2;
  if (!onext)
    return false;

  GenericObject gfut(onext, val.rawValue());
  // Need a live shared_ptr for shared_from_this() to work.
  boost::shared_ptr<GenericObject> ao(&gfut, &hold<GenericObject*>);
  boost::function<void()> cb = boost::bind(futureAdapterGeneric<T>, val, promise);
  // Careful, gfut will die at the end of this block, but it is
  // stored in call data. So call must finish before we exit this block,
  // and thus must be synchronous.
  try
  {
    gfut.call<void>("_connect", cb);
    promise.setOnCancel(qi::bindWithFallback<void(const qi::Promise<T>&)>(boost::function<void()>(), static_cast<void(GenericObject::*)(const std::string&)>(&GenericObject::call<void>), boost::weak_ptr<GenericObject>(gfut.shared_from_this()), "cancel"));
  }
  catch (std::exception& e)
  {
    qiLogError("qi.object") << "future connect error " << e.what();
  }
  return true;
}

struct AutoRefDestroy
{
  AnyReference val;
  AutoRefDestroy(AnyReference ref) : val(ref) {}
  ~AutoRefDestroy()
  {
    val.destroy();
  }
};

template <typename T>
inline T extractFuture(qi::Future<qi::AnyReference> metaFut)
{
  AnyReference val =  metaFut.value();
  AutoRefDestroy destroy(val);

  static TypeInterface* targetType;
  QI_ONCE(targetType = typeOf<T>());
  try
  {
    std::pair<AnyReference, bool> conv = val.convert(targetType);
    if (!conv.first.type())
      throw std::runtime_error(std::string("Unable to convert call result to target type: from ")
        + val.signature(true).toPrettySignature() + " to " + targetType->signature().toPrettySignature());
    else
    {
      if (conv.second)
      {
        AutoRefDestroy destroy(conv.first);
        return *conv.first.ptr<T>(false);
      }
      else
        return *conv.first.ptr<T>(false);
    }
  }
  catch(const std::exception& e)
  {
    throw std::runtime_error(std::string("Return argument conversion error: ") + e.what());
  }
}

template <>
inline void extractFuture<void>(qi::Future<qi::AnyReference> metaFut)
{
  AnyReference val = metaFut.value();
  val.destroy();
}

template <typename T>
inline void futureAdapter(qi::Future<qi::AnyReference> metaFut, qi::Promise<T> promise)
{
  qiLogDebug("qi.object") << "futureAdapter " << qi::typeOf<T>()->infoString()<< ' ' << metaFut.hasError();
  //error handling
  if (metaFut.hasError()) {
    promise.setError(metaFut.error());
    return;
  }

  AnyReference val =  metaFut.value();
  if (handleFuture(val, promise))
    return;

  static TypeInterface* targetType;
  QI_ONCE(targetType = typeOf<T>());
  try
  {
    std::pair<AnyReference, bool> conv = val.convert(targetType);
    if (!conv.first.type())
      promise.setError(std::string("Unable to convert call result to target type: from ")
        + val.signature(true).toPrettySignature() + " to " + targetType->signature().toPrettySignature() );
    else
    {
      promise.setValue(*conv.first.ptr<T>(false));
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

template <>
inline void futureAdapter<void>(qi::Future<qi::AnyReference> metaFut, qi::Promise<void> promise)
{
  qiLogDebug("qi.object") << "futureAdapter void " << metaFut.hasError();
  //error handling
  if (metaFut.hasError()) {
    promise.setError(metaFut.error());
    return;
  }
  AnyReference val =  metaFut.value();
  if (handleFuture(val, promise))
    return;

  promise.setValue(0);
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

}
}

#endif

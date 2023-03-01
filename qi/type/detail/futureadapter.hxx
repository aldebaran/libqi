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

static const char* InvalidValueError = "value is invalid";
static const char* InvalidFutureError = "function returned an invalid future";

template<typename T> void setPromise(qi::Promise<T>& promise, const AnyValue& v)
{
  if (!v.isValid())
  {
    promise.setError(InvalidValueError);
    return;
  }

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

template<> inline void setPromise(qi::Promise<void>& promise, const AnyValue&)
{
  promise.setValue(0);
}

template<> inline void setPromise(qi::Promise<AnyValue>& promise, const AnyValue& val)
{
  promise.setValue(val);
}

template<> inline void setPromise(qi::Promise<AnyReference>& promise, const AnyValue& val)
{
  promise.setValue(val.clone());
}

template <typename T>
void futureAdapterGeneric(AnyReference val, qi::Promise<T> promise,
    boost::shared_ptr<GenericObject> ao)
{
  if (!val.isValid())
  {
    promise.setError(InvalidValueError);
    return;
  }

  QI_ASSERT(ao);
  qiLogDebug("qi.adapter") << "futureAdapter trigger";
  TypeOfTemplate<Future>* ft1 = QI_TEMPLATE_TYPE_GET(val.type(), Future);
  TypeOfTemplate<FutureSync>* ft2 = QI_TEMPLATE_TYPE_GET(val.type(), FutureSync);
  qiLogDebug("qi.adapter") << "isFuture " << val.type()->infoString() << ' ' << !!ft1 << ' ' << !!ft2;
  bool isvoid = false;
  if (ft1)
    isvoid = ft1->templateArgument()->kind() == TypeKind_Void;
  else if (ft2)
    isvoid = ft2->templateArgument()->kind() == TypeKind_Void;
  GenericObject& gfut = *ao;
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
    qiLogDebug("qi.adapter") << "futureAdapter: future canceled";
    promise.setCanceled();
    return;
  }
  qiLogDebug("qi.adapter") << "futureAdapter: future has value";
  AnyValue v = gfut.call<AnyValue>("value", 0);
  // For a Future<void>, value() gave us a void*
  if (isvoid)
    v = AnyValue(qi::typeOf<void>());
  qiLogDebug("qi.adapter") << (v.isValid() ? v.type()->infoString() : "<invalid AnyValue>");
  setPromise(promise, v);
  qiLogDebug("qi.adapter") << "Promise set";
}

// return a generic object pointing to the future referenced by val or null if val is not a future
// remember that you need a shared_ptr pointing on the genericobject so that it can work (shared_from_this)
inline boost::shared_ptr<GenericObject> getGenericFuture(AnyReference val, TypeKind* kind = 0)
{
  if (!val.isValid())
  {
    qiLogDebug("qi.adapter") << "getGenericFuture: Invalid value.";
    return boost::shared_ptr<GenericObject>();
  }

  TypeOfTemplate<Future>* ft1 = QI_TEMPLATE_TYPE_GET(val.type(), Future);
  TypeOfTemplate<FutureSync>* ft2 = QI_TEMPLATE_TYPE_GET(val.type(), FutureSync);
  ObjectTypeInterface* onext = NULL;
  qiLogDebug("qi.adapter") << "isFuture " << val.type()->infoString() << ' ' << !!ft1 << ' ' << !!ft2;
  if (ft1)
  {
    if (kind)
      *kind = ft1->templateArgument()->kind();
    onext = ft1;
  }
  else if (ft2)
  {
    if (kind)
      *kind = ft2->templateArgument()->kind();
    onext = ft2;
  }
  if (!onext)
    return boost::shared_ptr<GenericObject>();
  return boost::make_shared<GenericObject>(onext, val.rawValue());
}

// futureAdapter helper that detects and handles value of kind future
// return true if value was a future and was handled
// Takes ownership of the value if it returns true.
template <typename T>
inline bool handleFuture(AnyReference val, Promise<T> promise)
{
  boost::shared_ptr<GenericObject> ao = getGenericFuture(val);
  if (!ao)
    return false;

  // At this point, we are sure that the value holds a Future, so we take its ownership.
  UniqueAnyReference uval{ val };
  if (!ao->call<bool>("isValid"))
  {
    promise.setError(InvalidFutureError);
    return true;
  }

  // The callback participates in the ownerships of the future and its generic object that it
  // captures and releases these participations after the first call. Calling it can therefore have
  // a destructive effect.
  // TODO: Remove use of a shared_ptr once we can initialize captures of the lambda (C++14).
  auto spVal = std::make_shared<UniqueAnyReference>(std::move(uval));
  boost::function<void()> cb = [=]() mutable {
      if (!spVal || !(*spVal)->isValid() || !ao)
        throw std::logic_error{"Future is either invalid or has already been adapted."};

      // In the case that these copies of the shared_ptrs are the last, destroy the GenericObject
      // before the AnyReference as it depends on it.
      auto localSpVal = ka::exchange(spVal, nullptr);
      auto localAo = ka::exchange(ao, nullptr);
      futureAdapterGeneric<T>(**localSpVal, promise, localAo);
    };

  const auto weakVal = ka::weak_ptr(ka::exchange(spVal, nullptr));
  const auto weakAo = ka::weak_ptr(ao);

  // Careful, gfut will die at the end of this block, but it is
  // stored in call data. So call must finish before we exit this block,
  // and thus must be synchronous.
  try
  {
    ao->call<void>("_connect", cb);

    promise.setOnCancel(
      [=](Promise<T>&) {
        // Keep the reference to the future alive until we're done using it as a GenericObject.
        if (auto val = weakVal.lock())
        {
          QI_IGNORE_UNUSED(val);
          if (auto g = weakAo.lock())
            g->call<void>("cancel");
        }
      }
    );
  }
  catch (std::exception& e)
  {
    qiLogError("qi.object") << "future connect error " << e.what();
    promise.setError("internal error: cannot connect returned future");
  }
  return true;
}

template <typename T>
inline T extractFuture(const qi::Future<qi::AnyReference>& metaFut)
{
  auto val = UniqueAnyReference{ metaFut.value() };
  if (!val->isValid())
    throw std::runtime_error(InvalidValueError);


  AnyValue hold;
  if (boost::shared_ptr<GenericObject> ao = getGenericFuture(*val))
  {
    if (!ao->call<bool>("isValid"))
      throw std::runtime_error(InvalidFutureError);

    hold = ao->call<qi::AnyValue>("value", (int)FutureTimeout_Infinite);
    *val = hold.asReference();
  }

  static TypeInterface* targetType;
  QI_ONCE(targetType = typeOf<T>());
  try
  {
    auto conv = val->convert(targetType);
    if (!conv->type())
      throw std::runtime_error(std::string("Unable to convert call result to target type: from ") +
                               val->signature(true).toPrettySignature() + " to " +
                               targetType->signature().toPrettySignature());
    return std::move(*conv->ptr<T>(false));
  }
  catch(const std::exception& e)
  {
    throw std::runtime_error(std::string("Return argument conversion error: ") + e.what());
  }
}

template <>
inline void extractFuture<void>(const qi::Future<qi::AnyReference>& metaFut)
{
  auto val = UniqueAnyReference{ metaFut.value() };
  if (!val->isValid())
    throw std::runtime_error(InvalidValueError);

  if (boost::shared_ptr<GenericObject> ao = getGenericFuture(*val))
  {
    if (!ao->call<bool>("isValid"))
      throw std::runtime_error(InvalidFutureError);

    ao->call<qi::AnyValue>("value", (int)FutureTimeout_Infinite);
  }
}

/// Factorization of the setting of the promise according to the future result.
/// This method takes ownership of the reference.
template <typename T>
inline void setAdaptedResult(Promise<T>& promise, UniqueAnyReference ref)
{
  if (!ref->isValid())
  {
    promise.setError(InvalidValueError);
    return;
  }

  static TypeInterface* targetType;
  QI_ONCE(targetType = typeOf<T>());
  try
  {
    auto conv = ref->convert(targetType);
    if (!conv->type())
      promise.setError(std::string("Unable to convert call result to target type: from ")
        + ref->signature(true).toPrettySignature() + " to " + targetType->signature().toPrettySignature() );
    else
    {
      promise.setValue(*conv->ptr<T>(false));
    }
  }
  catch(const std::exception& e)
  {
    promise.setError(std::string("Return argument conversion error: ") + e.what());
  }
}

/// Specialization for setting the promise according to the future result in the case of void.
/// This method takes ownership of the reference.
template <>
inline void setAdaptedResult<void>(Promise<void>& promise, UniqueAnyReference ref)
{
  if (!ref->isValid())
  {
    promise.setError(InvalidValueError);
    return;
  }

  promise.setValue(nullptr);
}

/// Specialization for setting the promise according to the future result in the case of AnyReference.
/// This method takes ownership of the reference.
template <>
inline void setAdaptedResult<AnyReference>(Promise<AnyReference>& promise, UniqueAnyReference ref)
{
  if (!ref->isValid())
  {
    promise.setError(InvalidValueError);
    return;
  }

  try
  {
    promise.setValue(ref->clone());
  }
  catch(const std::exception& e)
  {
    promise.setError(std::string("Return argument conversion error: ") + e.what());
  }
}

template <typename T>
inline void futureAdapter(const qi::Future<qi::AnyReference>& metaFut, qi::Promise<T> promise)
{
  //error handling
  if (metaFut.hasError()) {
    promise.setError(metaFut.error());
    return;
  }
  if (metaFut.isCanceled()) {
    promise.setCanceled();
    return;
  }

  auto val = metaFut.value();
  if (handleFuture(val, promise)) // Takes ownership of the value if it returns true.
    return;
  setAdaptedResult(promise, UniqueAnyReference{ val });
}

template <typename T>
inline void futureAdapterVal(const qi::Future<qi::AnyValue>& metaFut, qi::Promise<T> promise)
{
  //error handling
  if (metaFut.hasError()) {
    promise.setError(metaFut.error());
    return;
  }
  if (metaFut.isCanceled()) {
    promise.setCanceled();
    return;
  }
  const AnyValue& val =  metaFut.value();
  if (!val.isValid())
  {
    promise.setError(InvalidValueError);
    return;
  }

  try
  {
    promise.setValue(val.to<T>());
  }
  catch (const std::exception& e)
  {
    promise.setError(std::string("Return argument conversion error: ") + e.what());
  }
}

template <>
inline void futureAdapterVal(const qi::Future<qi::AnyValue>& metaFut, qi::Promise<AnyValue> promise)
{
  if (metaFut.hasError())
    promise.setError(metaFut.error());
  else if (metaFut.isCanceled())
    promise.setCanceled();
  else
    promise.setValue(metaFut.value());
}

template <>
inline void futureAdapterVal(const qi::Future<qi::AnyValue>& metaFut, qi::Promise<void> promise)
{
  if (metaFut.hasError())
    promise.setError(metaFut.error());
  else if (metaFut.isCanceled())
    promise.setCanceled();
  else
    promise.setValue(0);
}

}
}

#endif

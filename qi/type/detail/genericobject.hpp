#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_DETAIL_GENERIC_OBJECT_HPP_
#define _QI_TYPE_DETAIL_GENERIC_OBJECT_HPP_

#include <map>
#include <string>
#include <sstream>

#include <boost/smart_ptr/enable_shared_from_this.hpp>

#include <qi/api.hpp>
#include <qi/type/detail/futureadapter.hpp>
#include <qi/type/detail/manageable.hpp>
#include <qi/future.hpp>
#include <qi/signal.hpp>
#include <qi/type/typeobject.hpp>
#include <qi/objectuid.hpp>
#include <ka/macro.hpp>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )

namespace qi
{

/* ObjectValue
 *  static version wrapping class C: Type<C>
 *  dynamic version: Type<DynamicObject>
 *
 * All the methods are convenience wrappers that bounce to the ObjectTypeInterface,
 * except Event Loop management
 * This class has pointer semantic. Do not use directly, use AnyObject,
 * obtained through Session, DynamicObjectBuilder or ObjectTypeBuilder.
 */
class QI_API GenericObject
  : public Manageable
  , public boost::enable_shared_from_this<GenericObject>
{
public:
  GenericObject(ObjectTypeInterface *type, void *value, const boost::optional<ObjectUid>& maybeUid = boost::none);
  ~GenericObject();
  const MetaObject &metaObject();

  // Help doxygen and the header reader a bit.
  template <typename R, typename... Args>
  R call(const std::string& methodName, Args&&... args);

  template <typename R, typename... Args>
  qi::Future<R> async(const std::string& methodName, Args&&... args);

  /**
   * Call a method dynamically, using an extensible list of arguments and
   * specific call policies. Since the underlying call may be asynchronous,
   * these functions always return a future, which would match the underlying
   * future result. In other words, it is already unwrapped.
   */
  //@{
  /**
   * Call a method by its name or signature to deduce the method ID.
   * @param nameWithOptionalSignature method name or method signature
   * 'name::(args)' if signature is given, an exact match is required.
   * @param params arguments to pass to the call.
   * @param callType type of the call.
   * @param returnSignature forces the return type if set.
   */
  qi::Future<AnyReference> metaCall(const std::string &nameWithOptionalSignature, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto, Signature returnSignature = Signature());

  /**
   * Call a method dynamically, by method ID.
   * @param method method ID.
   * @param params arguments to pass to the call.
   * @param callType type of the call.
   * @param returnSignature forces the return type if set.
   */
  qi::Future<AnyReference> metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto, Signature returnSignature = Signature());
  //@}

  /// Find method named name callable with arguments parameters
  int findMethod(const std::string& name, const GenericFunctionParameters& parameters);

  void post(const std::string& eventName,
            qi::AutoAnyReference p1 = qi::AutoAnyReference(),
            qi::AutoAnyReference p2 = qi::AutoAnyReference(),
            qi::AutoAnyReference p3 = qi::AutoAnyReference(),
            qi::AutoAnyReference p4 = qi::AutoAnyReference(),
            qi::AutoAnyReference p5 = qi::AutoAnyReference(),
            qi::AutoAnyReference p6 = qi::AutoAnyReference(),
            qi::AutoAnyReference p7 = qi::AutoAnyReference(),
            qi::AutoAnyReference p8 = qi::AutoAnyReference());

  void metaPost(unsigned int event, const GenericFunctionParameters& params);
  void metaPost(const std::string &nameWithOptionalSignature, const GenericFunctionParameters &in);

  /** Connect an event to an arbitrary callback.
   *
   * If you are within a service, it is recommended that you connect the
   * event to one of your Slots instead of using this method.
   */
  template <typename FUNCTOR_TYPE>
  qi::FutureSync<SignalLink> connect(const std::string& eventName, FUNCTOR_TYPE callback,
                       MetaCallType threadingModel = MetaCallType_Direct);


  qi::FutureSync<SignalLink> connect(const std::string &name, const SignalSubscriber& functor);

  /// Calls given functor when event is fired. Takes ownership of functor.
  qi::FutureSync<SignalLink> connect(unsigned int signal, const SignalSubscriber& subscriber);

  /** Connect an event to a method.
   * Recommended use is when target is not a proxy.
   * If target is a proxy and this is server-side, the event will be
   *    registered localy and the call will be forwarded.
   * If target and this are proxies, the message will be routed through
   * the current process.
   */
  qi::FutureSync<SignalLink> connect(unsigned int signal, AnyObject target, unsigned int slot);

  /// Disconnect an event link. Returns if disconnection was successful.
  qi::FutureSync<void> disconnect(SignalLink linkId);

  template<typename T>
  qi::FutureSync<T> property(const std::string& name);

  template<typename T>
  qi::FutureSync<void> setProperty(const std::string& name, const T& val);

  //Low Level Properties
  qi::FutureSync<AnyValue> property(unsigned int id);
  qi::FutureSync<void> setProperty(unsigned int id, const AnyValue &val);


  bool isValid() { return type && value;}
  ObjectTypeInterface*  type;
  void*        value;
  ObjectUid uid; ///< Uid of "value".

private:
  /// Common meta call algorithm, without unwrapping the returned future.
  Future<AnyReference> metaCallNoUnwrap(
      unsigned int method,
      const GenericFunctionParameters& params,
      const MetaCallType callType,
      const Signature& returnSignature);

  /// Finds a method or throws a nicely formatted error message.
  std::string makeFindMethodErrorMessage(
      const std::string& nameWithOptionalSignature,
      const GenericFunctionParameters& args,
      const int errorNo);
};

namespace detail
{

// Storage type used by Object<T>, and Proxy.
using ManagedObjectPtr = boost::shared_ptr<GenericObject>;

}

// C4251
template <typename FUNCTION_TYPE>
qi::FutureSync<SignalLink> GenericObject::connect(const std::string& eventName,
                                                    FUNCTION_TYPE callback,
                                                    MetaCallType model)
{
  return connect(eventName,
    SignalSubscriber(AnyFunction::from(callback), model));
}

namespace detail
{

template <typename T>
struct isFuture : boost::false_type {};
template <typename T>
struct isFuture<qi::Future<T> > : boost::true_type {};
template <typename T>
struct isFuture<qi::FutureSync<T> > : boost::true_type {};

}

/* Generate R GenericObject::call(methodname, args...)
 * for all argument counts
 * The function packs arguments in a vector<AnyReference>, computes the
 * signature and bounce those to metaCall.
 */

template <typename R, typename... Args>
R GenericObject::call(const std::string& methodName, Args&&... args)
{
  static_assert(!detail::isFuture<R>::value, "return type of call must not be a Future");
  if (!value || !type)
    throw std::runtime_error("Invalid GenericObject");
  std::vector<qi::AnyReference> params = {qi::AnyReference::from(args)...};
  qi::Future<AnyReference> fmeta = metaCall(methodName, params, MetaCallType_Direct, typeOf<R>()->signature());
  return detail::extractFuture<R>(fmeta);
}

/// Calls a method of the generic object asynchronously.
/// @return a future tracking the result of the underlying method call.
/// If the underlying call returned a future, the future is unwrapped.
template <typename R, typename... Args>
qi::Future<R> GenericObject::async(const std::string& methodName, Args&&... args)
{
  std::vector<qi::AnyReference> anyArgs = {qi::AnyReference::from(args)...};
  int methodId = findMethod(methodName, anyArgs);
  if (methodId < 0) // in that case, the method ID is an error number
    return makeFutureError<R>(makeFindMethodErrorMessage(methodName, anyArgs, methodId));
  auto futureMeta = metaCallNoUnwrap(methodId, anyArgs, MetaCallType_Queued, typeOf<R>()->signature());
  qi::Promise<R> result;
  qi::adaptFutureUnwrap(futureMeta, result);
  return result.future();
}

template<typename T>
qi::FutureSync<T> GenericObject::property(const std::string& name)
{
  int pid = metaObject().propertyId(name);
  if (pid < 0)
  {
    std::ostringstream ss;
    ss << "property \"" << name << "\" was not found";
    return makeFutureError<T>(ss.str());
  }
  qi::Future<AnyValue> f = property(pid);
  qi::Promise<T> p;
  f.connect(boost::bind(&detail::futureAdapterVal<T>,_1, p),
      FutureCallbackType_Sync);
  return p.future();
}

template<typename T>
qi::FutureSync<void> GenericObject::setProperty(const std::string& name, const T& val)
{
  int pid = metaObject().propertyId(name);
  if (pid < 0)
  {
    std::ostringstream ss;
    ss << "property \"" << name << "\" was not found";
    return makeFutureError<void>(ss.str());
  }
  return setProperty(pid, AnyValue::from(val));
}

/* An AnyObject is actually of a Dynamic type: The underlying TypeInterface*
 * is not allways the same.
 * Override backend shared_ptr<GenericObject>
*/
template<>
class QI_API TypeImpl<boost::shared_ptr<GenericObject>> :
  public DynamicTypeInterface
{
public:
  AnyReference get(void* storage) override
  {
    detail::ManagedObjectPtr* val = (detail::ManagedObjectPtr*)ptrFromStorage(&storage);
    AnyReference result;
    if (!*val)
    {
      return AnyReference();
    }
    return AnyReference((*val)->type, (*val)->value);
  }

  void set(void** storage, AnyReference source) override
  {
    qiLogCategory("qitype.object");
    detail::ManagedObjectPtr* val = (detail::ManagedObjectPtr*)ptrFromStorage(storage);

    if (!source.type())
      throw std::runtime_error("cannot set object from an invalid value");

    if (source.type()->info() == info())
    { // source is objectptr
      detail::ManagedObjectPtr* src = source.ptr<detail::ManagedObjectPtr>(false);
      if (!*src)
        qiLogWarning() << "NULL Object";
      *val = *src;
    }
    else if (source.kind() == TypeKind_Dynamic)
    { // try to dereference dynamic type in case it contains an object
      auto content = source.content();
      if (!content.isValid())
        throw std::runtime_error("cannot set object from an invalid dynamic value");
      set(storage, source.content());
    }
    else if (source.kind() == TypeKind_Object)
    { // wrap object in objectptr: we do not keep it alive,
      // but source type offers no tracking capability
      detail::ManagedObjectPtr op(new GenericObject(static_cast<ObjectTypeInterface*>(source.type()), source.rawValue()));
      *val = op;
    }
    else if (source.kind() == TypeKind_Pointer)
    {
      PointerTypeInterface* ptype = static_cast<PointerTypeInterface*>(source.type());
      // FIXME: find a way!
      if (ptype->pointerKind() == PointerTypeInterface::Shared)
        qiLogVerbose() << "Object will *not* track original shared pointer";
      set(storage, *source);
    }
    else if (source.kind() == TypeKind_Optional)
    {
      set(storage, source.content());
    }
    else
      throw std::runtime_error((std::string)"Cannot assign non-object " + source.type()->infoString() + " to Object");
  }

  using Methods = DefaultTypeImplMethods<detail::ManagedObjectPtr, TypeByPointerPOD<detail::ManagedObjectPtr>>;
  _QI_BOUNCE_TYPE_METHODS(Methods);
};

}

KA_WARNING_POP()

#endif

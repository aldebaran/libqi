#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_PROXY_REGISTER_HPP_
#define _QITYPE_DETAIL_PROXY_REGISTER_HPP_

namespace qi
{

/** A Proxy is the base class used by bouncer implementations of all
* interfaces.
*/
class QI_API Proxy : public boost::noncopyable
{
public:
  Proxy(AnyObject obj) : _obj(obj) {qiLogDebug("qitype.proxy") << "Initializing " << this;}
  Proxy() {}
  ~Proxy() { qiLogDebug("qitype.proxy") << "Finalizing on " << this;}
  Object<Empty> asObject() const;
protected:
  Object<Empty> _obj;
};

inline AnyObject Proxy::asObject() const
{
  qiLogDebug("qitype.proxy") << "asObject " << this << ' ' << &_obj.asT();
  return AnyObject(_obj);
}

/* A proxy instance can have members: signals and properties, inherited from interface.
* So it need a type of its own, we cannot pretend it's a AnyObject.
*/
class TypeProxy: public ObjectTypeInterface
{
public:
  /* We need a per-instance offset from effective type to Proxy.
   * Avoid code explosion by putting it per-instance
  */
  typedef boost::function<Proxy*(void*)> ToProxy;
  TypeProxy(ToProxy  toProxy)
  : toProxy(toProxy)
  {
  }
  virtual const MetaObject& metaObject(void* instance)
  {
    Proxy* ptr = toProxy(instance);
    return ptr->asObject().metaObject();
  }
  virtual qi::Future<AnyReference> metaCall(void* instance, AnyObject context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType, Signature returnSignature)
  {
    Proxy* ptr = toProxy(instance);
    return ptr->asObject().metaCall(method, params, callType, returnSignature);
  }
  virtual void metaPost(void* instance, AnyObject context, unsigned int signal, const GenericFunctionParameters& params)
  {
    Proxy* ptr = toProxy(instance);
    ptr->asObject().metaPost(signal, params);
  }
  virtual qi::Future<SignalLink> connect(void* instance, AnyObject context, unsigned int event, const SignalSubscriber& subscriber)
  {
    Proxy* ptr = toProxy(instance);
    return ptr->asObject().connect(event, subscriber);
  }
  virtual qi::Future<void> disconnect(void* instance, AnyObject context, SignalLink linkId)
  {
     Proxy* ptr = toProxy(instance);
     return ptr->asObject().disconnect(linkId);
  }
  virtual const std::vector<std::pair<TypeInterface*, int> >& parentTypes()
  {
    static std::vector<std::pair<TypeInterface*, int> > empty;
    return empty;
  }
  virtual qi::Future<AnyValue> property(void* instance, AnyObject context, unsigned int id)
  {
    Proxy* ptr = toProxy(instance);
    GenericObject* obj = ptr->asObject().asGenericObject();
    return obj->type->property(obj->value, context, id);
  }
  virtual qi::Future<void> setProperty(void* instance, AnyObject context, unsigned int id, AnyValue value)
  {
    Proxy* ptr = toProxy(instance);
    GenericObject* obj = ptr->asObject().asGenericObject();
    return obj->type->setProperty(obj->value, context, id, value);
  }
  typedef DefaultTypeImplMethods<Proxy> Methods;
  _QI_BOUNCE_TYPE_METHODS(Methods);
  ToProxy toProxy;
};

/* We limit usage of per-class type for generated proxies
 *to non-interface mode
 * where it is needed.
 */
template<typename T> struct TypeProxyWrapper: public TypeProxy
{
  typedef boost::function<Proxy*(void*)> ToProxy;
  TypeProxyWrapper(ToProxy  toProxy)
  : TypeProxy(toProxy)
  {
  }
  typedef DefaultTypeImplMethods<T> Methods;
  _QI_BOUNCE_TYPE_METHODS(Methods);
};
// Needed for legacy code
//template<> struct TypeImpl<Proxy>: public TypeProxy {};

namespace detail
{
  // FIXME: inline that in QI_REGISTER_PROXY_INTERFACE maybe
  template<typename ProxyImpl> Proxy* static_proxy_cast(void* storage)
  {
    return static_cast<Proxy*>((ProxyImpl*)storage);
  }

  template<typename ProxyImpl>
  TypeProxy* makeProxyInterfaceWrapper()
  {
    static TypeProxy * result = 0;
    if (!result)
      result = new TypeProxyWrapper<ProxyImpl>(&static_proxy_cast<ProxyImpl>);
    return result;
  }

  template<typename ProxyImpl>
  TypeProxy* makeProxyInterface()
  {
    static TypeProxy * result = 0;
    if (!result)
      result = new TypeProxy(&static_proxy_cast<ProxyImpl>);
    return result;
  }

  template<typename ProxyImpl>
  AnyReference makeProxy(AnyObject ptr)
  {
    boost::shared_ptr<ProxyImpl> sp(new ProxyImpl(ptr));
    return AnyReference::from(sp).clone();
  }
}

/** Register \p Proxy as a proxy class for interface \p Interface.
 * Required to allow the typesystem to construct an
 * Object<Interface> from an AnyObject.
 * Proxy must be constructible with an AnyObject as argument
 * @return unused value, present to ease registration at static initialisation
 */
template<typename Proxy, typename Interface>
bool registerProxyInterface()
{
  qiLogVerbose("qitype.type") << "ProxyInterface registration " << typeOf<Interface>()->infoString();
  // Runtime-register TypeInterface for Proxy, using ProxyInterface with
  // proper static_cast (from Proxy template to qi::Proxy) helper.
  registerType(typeid(Proxy), detail::makeProxyInterface<Proxy>());
  detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
  map[typeOf<Interface>()->info()] = boost::function<AnyReference(AnyObject)>(&detail::makeProxy<Proxy>);
  return true;
}

/** Register \p Proxy as a proxy class.
 * Required for bound methods to accept a ProxyPtr as argument
 * Proxy must be constructible with an AnyObject as argument
 * @return unused value, present to ease registration at static initialisation
 */
template<typename ProxyType>
bool registerProxy()
{
  /* In non-interface mode, we need one different registered type
   * per Proxy class, otherwise they will all land in the same
   * bucket of proxyGeneratorMap.
   * In interface mode this problem is absent because the TypeInfo
   * of the interface is used (not the one of the proxy).
  */
  registerType(typeid(ProxyType), detail::makeProxyInterfaceWrapper<ProxyType>());
  detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
  map[typeOf<ProxyType>()->info()] = boost::function<AnyReference(AnyObject)>(&detail::makeProxy<ProxyType>);
  return true;
}

}

#define QI_REGISTER_PROXY(Proxy)                           \
  namespace {                                              \
    static bool BOOST_PP_CAT(_qi_register_proxy_, Proxy) = \
        ::qi::registerProxy<Proxy>();                      \
  }

#define QI_REGISTER_PROXY_INTERFACE(Proxy, Interface)      \
  namespace {                                              \
    static bool BOOST_PP_CAT(_qi_register_proxy_, Proxy) = \
        ::qi::registerProxyInterface<Proxy, Interface>();  \
  }

#endif

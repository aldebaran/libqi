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
template<class InterfaceType, class ProxyType>
class TypeProxy: public ObjectTypeInterface
{
public:
  /* We need a per-instance offset from effective type to Proxy.
   * Avoid code explosion by putting it per-instance
  */
  using ToProxy = boost::function<Proxy*(void*)>;
  TypeProxy(ToProxy  toProxy)
  : toProxy(toProxy)
  {
  }
  const MetaObject& metaObject(void* instance) override
  {
    Proxy* ptr = toProxy(instance);
    return ptr->asObject().metaObject();
  }

  ObjectUid uid(void* instance) const override
  {
    QI_ASSERT_TRUE(instance);
    Proxy* ptr = toProxy(instance);
    return ptr->asObject().uid();
  }

  qi::Future<AnyReference> metaCall(void* instance, AnyObject /*context*/, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType, Signature returnSignature) override
  {
    Proxy* ptr = toProxy(instance);
    return ptr->asObject().metaCall(method, params, callType, returnSignature);
  }
  void metaPost(void* instance, AnyObject /*context*/, unsigned int signal, const GenericFunctionParameters& params) override
  {
    Proxy* ptr = toProxy(instance);
    ptr->asObject().metaPost(signal, params);
  }
  qi::Future<SignalLink> connect(void* instance, AnyObject /*context*/, unsigned int event, const SignalSubscriber& subscriber) override
  {
    Proxy* ptr = toProxy(instance);
    return ptr->asObject().connect(event, subscriber);
  }
  qi::Future<void> disconnect(void* instance, AnyObject /*context*/, SignalLink linkId) override
  {
     Proxy* ptr = toProxy(instance);
     return ptr->asObject().disconnect(linkId);
  }
  const std::vector<std::pair<TypeInterface*, std::ptrdiff_t> >& parentTypes() override
  {
    using ReturnType = typename std::decay<decltype(parentTypes())>::type;
    static ReturnType* parents = nullptr;

    static const auto init = []{ return new ReturnType{
        { qi::typeOf<InterfaceType>(), []{
            ProxyType* ptr = static_cast<ProxyType*>(reinterpret_cast<void*>(0x10000));
            InterfaceType* pptr = ptr;
            std::ptrdiff_t offset = reinterpret_cast<intptr_t>(pptr)-reinterpret_cast<intptr_t>(ptr);
            return offset;
          }()
        }
      };
    };

    QI_ONCE(parents = init());

    return *parents;
  }
  qi::Future<AnyValue> property(void* instance, AnyObject context, unsigned int id) override
  {
    Proxy* ptr = toProxy(instance);
    GenericObject* obj = ptr->asObject().asGenericObject();
    return obj->type->property(obj->value, context, id);
  }
  qi::Future<void> setProperty(void* instance, AnyObject context, unsigned int id, AnyValue value) override
  {
    Proxy* ptr = toProxy(instance);
    GenericObject* obj = ptr->asObject().asGenericObject();
    return obj->type->setProperty(obj->value, context, id, value);
  }
  using Methods = DefaultTypeImplMethods<Proxy>;
  _QI_BOUNCE_TYPE_METHODS(Methods);
  ToProxy toProxy;
};

namespace detail
{
  // FIXME: inline that in QI_REGISTER_PROXY_INTERFACE maybe
  template<typename ProxyImpl> Proxy* static_proxy_cast(void* storage)
  {
    return static_cast<Proxy*>((ProxyImpl*)storage);
  }

  template<class InterfaceType, typename ProxyImpl>
  TypeProxy<InterfaceType, ProxyImpl>* makeProxyInterface()
  {
    static TypeProxy<InterfaceType, ProxyImpl>* result = 0;
    if (!result)
      result = new TypeProxy<InterfaceType, ProxyImpl>(&static_proxy_cast<ProxyImpl>);
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
  registerType(qi::typeId<Proxy>(), detail::makeProxyInterface<Interface, Proxy>());
  detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
  map[typeOf<Interface>()->info()] = boost::function<AnyReference(AnyObject)>(&detail::makeProxy<Proxy>);
  return true;
}

}

#define QI_REGISTER_PROXY(Proxy)                                          \
  namespace {                                                             \
    static bool BOOST_PP_CAT(_qi_register_proxy_, Proxy) QI_ATTR_UNUSED = \
        ::qi::registerProxy<Proxy>();                                     \
  }

#define QI_REGISTER_PROXY_INTERFACE(Proxy, Interface)                     \
  namespace {                                                             \
    static bool BOOST_PP_CAT(_qi_register_proxy_, Proxy) QI_ATTR_UNUSED = \
        ::qi::registerProxyInterface<Proxy, Interface>();                 \
  }

#endif

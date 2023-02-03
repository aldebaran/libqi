/*
**  Copyright (C) 2012-2014 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/type_traits/is_signed.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/anyobject.hpp>
#include <qi/session.hpp>
#include <qi/path.hpp>
#include <ka/empty.hpp>
#include <ka/src.hpp>

#include "type/metaobject_p.hpp"
#include "type/metamethod_p.hpp"
#include "messaging/serviceinfo_p.hpp"

namespace qi {

#define INTEGRAL_TYPE(t) \
static bool BOOST_PP_CAT(unused_ , __LINE__) QI_ATTR_UNUSED \
  = registerType(qi::typeId<t>(), new IntTypeInterfaceImpl<t>());

/** Integral types.
 * Since long is neither int32 nor uint32 on 32 bit platforms,
 * use all known native types instead of size/signedness explicit
 * types.
 */
INTEGRAL_TYPE(char);
INTEGRAL_TYPE(signed char);
INTEGRAL_TYPE(unsigned char);
INTEGRAL_TYPE(short);
INTEGRAL_TYPE(unsigned short);
INTEGRAL_TYPE(int);
INTEGRAL_TYPE(unsigned int);
INTEGRAL_TYPE(long);
INTEGRAL_TYPE(unsigned long);
INTEGRAL_TYPE(long long);
INTEGRAL_TYPE(unsigned long long);
}

QI_TYPE_REGISTER_CUSTOM(bool, qi::TypeBoolImpl<bool>);




namespace qi {

#define FLOAT_TYPE(t) \
static bool BOOST_PP_CAT(unused_ , __LINE__) QI_ATTR_UNUSED \
  = registerType(qi::typeId<t>(), new FloatTypeInterfaceImpl<t>());

FLOAT_TYPE(float);
FLOAT_TYPE(double);
}


#include <qi/clock.hpp>

template<typename T>
class DurationTypeInterface: public qi::IntTypeInterface
{
public:
  using ImplType = qi::DefaultTypeImplMethods<T, qi::TypeByPointerPOD<T>>;

  int64_t get(void* value) override
  {
    return boost::chrono::duration_cast<qi::Duration>(*((T*)ImplType::Access::ptrFromStorage(&value))).count();
  }

  void set(void** storage, int64_t value) override
  {
    (*(T*)ImplType::Access::ptrFromStorage(storage)) = boost::chrono::duration_cast<T>(qi::Duration(value));
  }

  unsigned int size() override
  {
    return sizeof(qi::int64_t);
  }

  bool isSigned() override
  {
    return false;
  }

  _QI_BOUNCE_TYPE_METHODS(ImplType);
};

template <typename T>
class TimePointTypeInterface: public qi::IntTypeInterface
{
public:
  using ImplType = qi::DefaultTypeImplMethods<T, qi::TypeByPointerPOD<T>>;
  int64_t get(void* value) override
  {
    T* tp = (T*)ImplType::Access::ptrFromStorage(&value);
    return tp->time_since_epoch().count();
  }

  void set(void** storage, int64_t value) override
  {
    T* tp = (T*)ImplType::Access::ptrFromStorage(storage);
    *tp = T(qi::Duration(value));
  }

  unsigned int size() override
  {
    return sizeof(qi::int64_t);
  }

  bool isSigned() override
  {
    return false;
  }

  _QI_BOUNCE_TYPE_METHODS(ImplType);
};


QI_EQUIVALENT_STRING_REGISTER(qi::Signature, &qi::Signature::toString);
QI_EQUIVALENT_STRING_REGISTER(qi::Path, &qi::Path::str);

QI_TYPE_REGISTER_CUSTOM(qi::Duration, DurationTypeInterface<qi::Duration>);
QI_TYPE_REGISTER_CUSTOM(qi::NanoSeconds, DurationTypeInterface<qi::NanoSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::MicroSeconds, DurationTypeInterface<qi::MicroSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::MilliSeconds, DurationTypeInterface<qi::MilliSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::Seconds, DurationTypeInterface<qi::Seconds>);
QI_TYPE_REGISTER_CUSTOM(qi::Minutes, DurationTypeInterface<qi::Minutes>);
QI_TYPE_REGISTER_CUSTOM(qi::Hours, DurationTypeInterface<qi::Hours>);


QI_TYPE_REGISTER_CUSTOM(qi::SystemClockTimePoint, TimePointTypeInterface<qi::SystemClockTimePoint>);
QI_TYPE_REGISTER_CUSTOM(qi::ClockTimePoint, TimePointTypeInterface<qi::ClockTimePoint>);
QI_TYPE_REGISTER_CUSTOM(qi::SteadyClockTimePoint, TimePointTypeInterface<qi::SteadyClockTimePoint>);

#define PBOUNCE(cls, field, type) \
  static const type& QI_CAT(QI_CAT(cls, _), field)(::qi::cls* ptr) { \
    return ptr->_p->field; \
  }

namespace {
PBOUNCE(MetaMethod, uid,         unsigned int)
PBOUNCE(MetaMethod, name,        std::string)
PBOUNCE(MetaMethod, description, std::string)
PBOUNCE(MetaMethod, parameters,  ::qi::MetaMethodParameterVector)
PBOUNCE(MetaMethod, returnDescription, std::string)

PBOUNCE(MetaMethodParameter, name,        std::string)
PBOUNCE(MetaMethodParameter, description, std::string)

  static const qi::MetaObject::MethodMap& methodMap(qi::MetaObject* ptr)
  {
    return ptr->_p->_methods;
  }
  static const qi::MetaObject::SignalMap& signalMap(qi::MetaObject* ptr)
  {
    return ptr->_p->_events;
  }
  static const qi::MetaObject::PropertyMap& propertyMap(qi::MetaObject* ptr)
  {
    return ptr->_p->_properties;
  }
  static const std::string& description(qi::MetaObject* ptr)
  {
    return ptr->_p->_description;
  }
}

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_REGISTER(::qi::MetaMethodParameter,
  QI_STRUCT_HELPER("name", MetaMethodParameter_name),
  QI_STRUCT_HELPER("description", MetaMethodParameter_description));

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_REGISTER(::qi::MetaMethod,
  QI_STRUCT_HELPER("uid", MetaMethod_uid),
  ("returnSignature", returnSignature),
  QI_STRUCT_HELPER("name", MetaMethod_name),
  ("parametersSignature", parametersSignature),
  QI_STRUCT_HELPER("description", MetaMethod_description),
  QI_STRUCT_HELPER("parameters", MetaMethod_parameters),
  QI_STRUCT_HELPER("returnDescription", MetaMethod_returnDescription));

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_REGISTER(::qi::MetaObject,
  QI_STRUCT_HELPER("methods", methodMap),
  QI_STRUCT_HELPER("signals", signalMap),
  QI_STRUCT_HELPER("properties", propertyMap),
  QI_STRUCT_HELPER("description", description));

namespace qi{
namespace {

  class ObjectUidTypeInterface : public RawTypeInterface
  {
  public:

    std::pair<char*, size_t> get(void* storage) override
    {
      auto& uid = *static_cast<ObjectUid*>(Methods::ptrFromStorage(&storage));
      return { reinterpret_cast<char*>(begin(uid)), size(uid) };
    }

    void set(void** storage, const char* ptr, size_t sz) override
    {
      auto& uid = *static_cast<ObjectUid*>(ptrFromStorage(storage));
      std::copy(ptr, ptr + sz, begin(uid));
    }
    using Methods = DefaultTypeImplMethods<ObjectUid, TypeByPointerPOD<ObjectUid> >;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

}}

QI_TYPE_REGISTER_CUSTOM(qi::ObjectUid, qi::ObjectUidTypeInterface);

static qi::ServiceInfoPrivate* serviceInfoPrivate(qi::ServiceInfo* svcinfo) {
    return svcinfo->_p;
}
QI_EQUIVALENT_STRING_REGISTER(qi::Url, &qi::Url::str);

namespace
{
  qi::Uri uriOrThrow(const std::string& str)
  {
    auto res = qi::uri(str);
    if (ka::empty(res))
      throw std::runtime_error("URI parsing error: '" + str + "' is not a valid URI.");
    return ka::src(std::move(res));
  }
}

QI_EQUIVALENT_STRING_REGISTER2(
  qi::Uri,
  static_cast<std::string (*)(const qi::Uri&)>(&qi::to_string),
  &::uriOrThrow);
QI_TYPE_STRUCT_EXTENSION_ADDED_FIELDS(qi::ServiceInfoPrivate, "objectUid");
QI_TYPE_STRUCT(qi::ServiceInfoPrivate, name, serviceId, machineId, processId, endpoints, sessionId, objectUid);
QI_TYPE_REGISTER(::qi::ServiceInfoPrivate);

QI_TYPE_STRUCT_BOUNCE_REGISTER(::qi::ServiceInfo, ::qi::ServiceInfoPrivate, serviceInfoPrivate);

static qi::AnyReference sessionLoadService(qi::AnyReferenceVector args)
{
  if (args.size() < 3)
    throw std::runtime_error("Not enough arguments");
  qi::Session& session = args[0].as<qi::Session>();
  std::string module = args[1].toString();
  std::string rename = args[2].toString();
  args.erase(args.begin(), args.begin()+3);
  session.loadService(module, rename, args);
  return qi::AnyReference(qi::typeOf<void>());
}

static qi::AnyReference sessionCallModule(qi::AnyReferenceVector args)
{
  if (args.size() < 2)
    throw std::runtime_error("Not enough arguments");
  qi::Session& session = args[0].as<qi::Session>();
  std::string module = args[1].toString();
  args.erase(args.begin(), args.begin()+2);
  auto fut = new qi::Future<qi::AnyValue>(session.callModule<qi::AnyValue>(module, args).async());
  return qi::AnyReference(qi::typeOf<decltype(*fut)>(), fut);
}

static qi::Future<std::vector<qi::ServiceInfo> > servicesBouncer(qi::Session& session)
{
  return session.services();
}

namespace qi {
static qi::AnyReference sessionSetClientAuthenticatorFactory(qi::AnyReferenceVector args)
{
  class DynamicClientAuth : public ClientAuthenticator
  {
  public:
    DynamicClientAuth(qi::AnyObject obj)
      : obj_(obj) {}

    CapabilityMap initialAuthData() override
    {
      return obj_.call<CapabilityMap>("initialAuthData");
    }

    CapabilityMap _processAuth(const CapabilityMap& authData) override
    {
      return obj_.call<CapabilityMap>("_processAuth", authData);
    }

  private:
    AnyObject obj_;
  };

  class DynamicClientAuthFactory : public ClientAuthenticatorFactory
  {
  public:
    DynamicClientAuthFactory(qi::AnyObject obj)
      : obj_(obj) {}

    ClientAuthenticatorPtr newAuthenticator() override
    {
      auto authenticator = obj_.call<AnyObject>("newAuthenticator");
      return boost::make_shared<DynamicClientAuth>(authenticator);
    }

  private:
    AnyObject obj_;
  };

  if (args.size() < 2)
    throw std::runtime_error("Not enough arguments");
  qi::Session& session = args[0].as<qi::Session>();
  auto factory = args[1].toObject();
  if (!factory)
  {
    throw std::runtime_error("Invalid Factory");
  }

  auto sharedFactory = boost::make_shared<DynamicClientAuthFactory>(factory);
  session.setClientAuthenticatorFactory(sharedFactory);
  return qi::AnyReference(qi::typeOf<void>());
}
} // qi

static bool _qiregisterSession() {
  ::qi::ObjectTypeBuilder<qi::Session> builder;
  builder.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, connect, qi::FutureSync<void>, ());
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, connect, qi::FutureSync<void>, (const std::string&));
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, isConnected);
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, url);
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, services);
  builder.advertiseMethod("services", &servicesBouncer);
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, service, qi::FutureSync<qi::AnyObject>, (const std::string&, const std::string&));
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, service, qi::FutureSync<qi::AnyObject>, (const std::string&, const std::string&, qi::MilliSeconds));
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, service, qi::FutureSync<qi::AnyObject>, (const std::string&));
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, service, qi::FutureSync<qi::AnyObject>, (const std::string&, qi::MilliSeconds));
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, listen, qi::FutureSync<void>, ());
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, listen, qi::FutureSync<void>, (const qi::Url&));
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, listen, qi::FutureSync<void>, (const std::vector<qi::Url>&));
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, endpoints);
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, close);
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, listenStandalone, qi::FutureSync<void>, ());
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, listenStandalone, qi::FutureSync<void>, (const qi::Url &));
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, listenStandalone, qi::FutureSync<void>, (const std::vector<qi::Url> &));
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, registerService);
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, unregisterService);
  // these two methods are variadic, make a dynamic bouncer
  builder.advertiseMethod("loadServiceRename", qi::AnyFunction::fromDynamicFunction(&sessionLoadService));
  builder.advertiseMethod("callModule", qi::AnyFunction::fromDynamicFunction(&sessionCallModule));
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, waitForService, qi::FutureSync<void>, (const std::string&, qi::MilliSeconds));
  QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, qi::Session, waitForService, qi::FutureSync<void>, (const std::string&));
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, serviceRegistered);
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, serviceUnregistered);
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, connected);
  QI_OBJECT_BUILDER_ADVERTISE(builder, qi::Session, disconnected);

  // We don't want to expose the hierarchy of ClientAuthenticator. So we create a function, setClientAuthenticatorFactory
  // which takes an array of AnyReference, and internally convert them to ClientAuthenticator.
  builder.advertiseMethod("setClientAuthenticatorFactory", qi::AnyFunction::fromDynamicFunction(&qi::sessionSetClientAuthenticatorFactory));

  builder.registerType();
  return true;
}
static bool _qi_registrationSession QI_ATTR_UNUSED = _qiregisterSession();

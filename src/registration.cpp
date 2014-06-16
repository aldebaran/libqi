/*
**  Copyright (C) 2012-2014 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/type_traits/is_signed.hpp>
#include <qitype/typeinterface.hpp>
#include <qi/anyobject.hpp>

#include "metaobject_p.hpp"
#include "metamethod_p.hpp"

namespace qi {

#define INTEGRAL_TYPE(t) \
static bool BOOST_PP_CAT(unused_ , __LINE__) = registerType(typeid(t), new IntTypeInterfaceImpl<t>());

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
static bool BOOST_PP_CAT(unused_ , __LINE__) = registerType(typeid(t), new FloatTypeInterfaceImpl<t>());

FLOAT_TYPE(float);
FLOAT_TYPE(double);
}


#include <qi/clock.hpp>

template<typename T>
class DurationTypeInterface: public qi::IntTypeInterface
{
public:
  typedef qi::DefaultTypeImplMethods<T, qi::TypeByPointerPOD<T> > ImplType;

  virtual int64_t get(void* value)
  {
    return boost::chrono::duration_cast<qi::Duration>(*((T*)ImplType::Access::ptrFromStorage(&value))).count();
  }

  virtual void set(void** storage, int64_t value)
  {
    (*(T*)ImplType::Access::ptrFromStorage(storage)) = boost::chrono::duration_cast<T>(qi::Duration(value));
  }

  virtual unsigned int size()
  {
    return sizeof(qi::int64_t);
  }

  virtual bool isSigned()
  {
    return false;
  }

  _QI_BOUNCE_TYPE_METHODS(ImplType);
};

template <typename T>
class TimePointTypeInterface: public qi::IntTypeInterface
{
public:
  typedef qi::DefaultTypeImplMethods<T, qi::TypeByPointerPOD<T> > ImplType;
  virtual int64_t get(void* value)
  {
    T* tp = (T*)ImplType::Access::ptrFromStorage(&value);
    return tp->time_since_epoch().count();
  }

  virtual void set(void** storage, int64_t value)
  {
    T* tp = (T*)ImplType::Access::ptrFromStorage(storage);
    *tp = T(qi::Duration(value));
  }

  virtual unsigned int size()
  {
    return sizeof(qi::int64_t);
  }

  virtual bool isSigned()
  {
    return false;
  }

  _QI_BOUNCE_TYPE_METHODS(ImplType);
};


QI_EQUIVALENT_STRING_REGISTER(qi::Signature, &qi::Signature::toString);

QI_TYPE_REGISTER_CUSTOM(qi::Duration, DurationTypeInterface<qi::Duration>);
QI_TYPE_REGISTER_CUSTOM(qi::NanoSeconds, DurationTypeInterface<qi::NanoSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::MicroSeconds, DurationTypeInterface<qi::MicroSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::MilliSeconds, DurationTypeInterface<qi::MilliSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::Seconds, DurationTypeInterface<qi::Seconds>);
QI_TYPE_REGISTER_CUSTOM(qi::Minutes, DurationTypeInterface<qi::Minutes>);
QI_TYPE_REGISTER_CUSTOM(qi::Hours, DurationTypeInterface<qi::Hours>);


QI_TYPE_REGISTER_CUSTOM(qi::WallClockTimePoint, TimePointTypeInterface<qi::WallClockTimePoint>);
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

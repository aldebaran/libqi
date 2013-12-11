/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/type_traits/is_signed.hpp>
#include <qitype/typeinterface.hpp>

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
  typedef typename qi::detail::TypeImplMethodsBySize<T>::type ImplType;

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
  typedef typename qi::detail::TypeImplMethodsBySize<T>::type ImplType;

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



QI_TYPE_REGISTER_CUSTOM(qi::Duration, DurationTypeInterface<qi::Duration>);
QI_TYPE_REGISTER_CUSTOM(qi::NanoSeconds, DurationTypeInterface<qi::NanoSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::MicroSeconds, DurationTypeInterface<qi::MicroSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::MilliSeconds, DurationTypeInterface<qi::MilliSeconds>);
QI_TYPE_REGISTER_CUSTOM(qi::Seconds, DurationTypeInterface<qi::Seconds>);
QI_TYPE_REGISTER_CUSTOM(qi::Minutes, DurationTypeInterface<qi::Minutes>);
QI_TYPE_REGISTER_CUSTOM(qi::Hours, DurationTypeInterface<qi::Hours>);


QI_TYPE_REGISTER_CUSTOM(qi::WallClockTimePoint, TimePointTypeInterface<qi::WallClockTimePoint>);
QI_TYPE_REGISTER_CUSTOM(qi::SteadyClockTimePoint, TimePointTypeInterface<qi::SteadyClockTimePoint>);

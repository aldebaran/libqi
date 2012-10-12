/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/type_traits/is_signed.hpp>

#include <qitype/type.hpp>
#include <qitype/typespecialized.hpp>
#include <qitype/genericvaluespecialized.hpp>

namespace qi {

template<typename T> class TypeIntImpl:
  public TypeInt
{
public:
  typedef typename detail::TypeImplMethodsBySize<T>::type
   ImplType;
  virtual int64_t get(void* value) const
  {
    return *(T*)ImplType::Access::ptrFromStorage(&value);
  }
  virtual void set(void** storage, int64_t value)
  {
    *(T*)ImplType::Access::ptrFromStorage(storage) = (T)value;
  }
  virtual unsigned int size() const
  {
    return sizeof(T);
  }
  virtual bool isSigned() const
  {
    return boost::is_signed<T>::value;
  }
  _QI_BOUNCE_TYPE_METHODS(ImplType);
};

// bool
template<> class TypeImpl<bool>: public TypeIntImpl<char>{};
// Force 64bit long
template<> class TypeIntImpl<long>: public TypeIntImpl<long long>{};
template<> class TypeIntImpl<unsigned long>: public TypeIntImpl<unsigned long long>{};

#define INTEGRAL_TYPE(t) \
static bool BOOST_PP_CAT(unused_ , __LINE__) = registerType(typeid(t), new TypeIntImpl<t>());

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

QI_TYPE_REGISTER_CUSTOM(bool, qi::TypeIntImpl<char>);




namespace qi {


template<typename T> class TypeFloatImpl: public TypeFloat
{
public:
  typedef typename detail::TypeImplMethodsBySize<T>::type
  ImplType;
  virtual double get(void* value) const
  {
    return *(T*)ImplType::Access::ptrFromStorage(&value);
  }
  virtual void set(void** storage, double value)
  {
    *(T*)ImplType::Access::ptrFromStorage(storage) = (T)value;
  }
  virtual unsigned int size() const
  {
    return sizeof(T);
  }
  _QI_BOUNCE_TYPE_METHODS(ImplType);
};

#define FLOAT_TYPE(t) \
static bool BOOST_PP_CAT(unused_ , __LINE__) = registerType(typeid(t), new TypeFloatImpl<t>());

FLOAT_TYPE(float);
FLOAT_TYPE(double);
}


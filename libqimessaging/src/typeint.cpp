#include <qimessaging/typeint.hpp>

namespace qi {

template<typename T> class TypeIntImpl: public virtual TypeInt,
public virtual DefaultTypeImpl<T,
    TypeDirectAccess<T>,
    TypeNoClone<TypeDirectAccess<T> >,
    TypeDefaultValue<TypeDirectAccess<T> >,
    TypeDefaultSerialize<TypeDirectAccess<T> >
    >
{
public:
  virtual int64_t get(void* value)
  {
    return *(T*)value;
  }
};

#define INTEGRAL_TYPE(t) \
static bool BOOST_PP_CAT(unused_ , __LINE__) = registerType(typeid(t), new TypeIntImpl<t>());

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

QI_REGISTER_MAPPING("i", qi::int32_t);
QI_REGISTER_MAPPING("I", qi::uint32_t);

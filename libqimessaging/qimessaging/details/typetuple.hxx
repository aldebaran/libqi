#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPETUPLE_HXX_
#define _QIMESSAGING_TYPETUPLE_HXX_

#include <qi/preproc.hpp>

namespace qi
{
  namespace detail {
    template<typename T> void setFromStorage(T& ref, void* storage)
    {
      ref = *(T*)typeOf<T>()->ptrFromStorage(&storage);
    }
  }
}

#define QI_TYPE_STRUCT_DECLARE(name)                                      \
namespace qi {                                                            \
  template<> struct TypeImpl<name>: public TypeTuple                      \
  {                                                                       \
  public:                                                                 \
    virtual std::vector<Type*> memberTypes(void*);                        \
    virtual void* get(void* storage, unsigned int index);                 \
    virtual void set(void** storage, unsigned int index, void* valStorage); \
    _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<name>);                \
 }; }


#define __QI_TUPLE_TYPE(_, what, field) res.push_back(typeOf(ptr->field));
#define __QI_TUPLE_GET(_, what, field) if (i == index) return typeOf(ptr->field)->initializeStorage(&ptr->field); i++;
#define __QI_TUPLE_SET(_, what, field) if (i == index) detail::setFromStorage(ptr->field, valueStorage); i++;
#define __QI_TYPE_STRUCT_IMPLEMENT(name, inl, onSet, ...)                                    \
namespace qi {                                                                        \
  inl std::vector<Type*> TypeImpl<name>::memberTypes(void* storage)                   \
  {                                                                                   \
    name* ptr = 0;                                                                    \
    std::vector<Type*> res;                                                           \
    QI_VAARGS_APPLY(__QI_TUPLE_TYPE, _, __VA_ARGS__);                                 \
    return res;                                                                       \
  }                                                                                   \
  inl void* TypeImpl<name>::get(void* storage, unsigned int index)                    \
  {                                                                                   \
    unsigned int i = 0;                                                                        \
    name* ptr = (name*)ptrFromStorage(&storage);                                      \
    QI_VAARGS_APPLY(__QI_TUPLE_GET, _, __VA_ARGS__);                                  \
    return 0;                                                                         \
  }                                                                                   \
  inl void TypeImpl<name>::set(void** storage, unsigned int index, void* valueStorage)\
  {                                                                                   \
    unsigned int i=0;                                                                 \
    name* ptr = (name*)ptrFromStorage(storage);                                       \
    QI_VAARGS_APPLY(__QI_TUPLE_SET, _, __VA_ARGS__);                                  \
    onSet                                                                      \
  }\
}

#define QI_TYPE_STRUCT_PRIVATE_ACCESS(name) \
friend class qi::TypeImpl<name>;

#define QI_TYPE_STRUCT(name, ...) \
  QI_TYPE_STRUCT_DECLARE(name) \
  __QI_TYPE_STRUCT_IMPLEMENT(name, inline, /**/, __VA_ARGS__)

#define QI_TYPE_STRUCT_EX(name, onSet, ...) \
  QI_TYPE_STRUCT_DECLARE(name) \
  __QI_TYPE_STRUCT_IMPLEMENT(name, inline, onSet, __VA_ARGS__)

#define QI_TYPE_STRUCT_IMPLEMENT(name, ...) \
  __QI_TYPE_STRUCT_IMPLEMENT(name, /**/, /**/, __VA_ARGS__)

#define QI_TYPE_STRUCT_BOUNCE(name, bounceTo, conversion)                 \
namespace qi {                                                            \
template<> class TypeImpl<name>: public TypeTupleBouncer<name, bounceTo>  \
{                                                                         \
public:                                                                   \
  void adaptStorage(void** storage, void** adapted)                       \
  {                                                                       \
    name* ptr = (name*)ptrFromStorage(storage);                           \
    bounceTo * tptr = conversion(ptr);                                    \
    *adapted = bounceType()->initializeStorage(tptr);                     \
  }                                                                       \
};}



namespace qi {
  template<typename T, typename TO> class TypeTupleBouncer: public TypeTuple
  {
  public:
    TypeTuple* bounceType()
    {
      static Type* result = 0;
      if (!result)
        result = typeOf<TO>();
      return static_cast<TypeTuple*>(result);
    }
    virtual void adaptStorage(void** storage, void** adapted) = 0;
    typedef DefaultTypeImplMethods<T> Methods;
    virtual std::vector<Type*> memberTypes(void* storage)
    {
      void* astorage = 0;
      if (storage) // memberTypes should not require storage
        adaptStorage(&storage, &astorage);
      return bounceType()->memberTypes(astorage);
    }
    virtual void* get(void* storage, unsigned int index)
    {
      void* astorage;
      adaptStorage(&storage, &astorage);
      return bounceType()->get(astorage, index);
    }
    virtual void set(void** storage, unsigned int index, void* valStorage)
    {
      void* astorage;
      adaptStorage(storage, &astorage);
      bounceType()->set(&astorage, index, valStorage);
    }
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };
}
#endif

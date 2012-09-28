#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPETUPLE_HXX_
#define _QIMESSAGING_TYPETUPLE_HXX_

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
#define __QI_TYPE_STRUCT_IMPLEMENT(name, inl, ...)                                    \
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
    unsigned int i=0;                                                                          \
    name* ptr = (name*)ptrFromStorage(storage);                                       \
    QI_VAARGS_APPLY(__QI_TUPLE_SET, _, __VA_ARGS__);                                  \
  }\
}


#define QI_TYPE_STRUCT(name, ...) \
  QI_TYPE_STRUCT_DECLARE(name) \
  __QI_TYPE_STRUCT_IMPLEMENT(name, inline, __VA_ARGS__)

#define QI_TYPE_STRUCT_IMPLEMENT(name, ...) \
  __QI_TYPE_STRUCT_IMPLEMENT(name, /**/, __VA_ARGS__)

#endif

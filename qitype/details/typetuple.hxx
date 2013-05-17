#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPETUPLE_HXX_
#define _QITYPE_DETAILS_TYPETUPLE_HXX_

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <qi/preproc.hpp>

namespace qi
{
  namespace detail {
    template<typename T> void setFromStorage(T& ref, void* storage)
    {
      ref = *(T*)typeOf<T>()->ptrFromStorage(&storage);
    }
    /* Overloads to manipulate an accessor: field, field getter, free function
    * We need given the accessor (and instance):
    * - access to erased type
    * - invocation
    * - acess to typed value (to invoke the setter)
    */
    template<typename C, typename T> Type* fieldType(T C::*field)
    {
      return typeOf<T>();
    }
    template<typename C, typename T> Type* fieldType(T (C::*field)(void))
    {
      return typeOf<T>();
    }
    template<typename C, typename T> Type* fieldType(T (C::*field)(void) const)
    {
      return typeOf<T>();
    }
    template<typename C, typename T> Type* fieldType(T (*field)(C*))
    {
      return typeOf<T>();
    }
    /* TypeTuple::get expects a pointer within storage, so a copy is not
    * acceptable. So force T& return type.
    *
    * Also, cl.exe does weird things and somehow match a member function
    * value to a member object type. So use enable_if to protect the
    * template.
    */
    template<typename C, typename T> void* fieldStorage(C* inst, T& (C::*field)(void))
    {
      return typeOf<T>()->initializeStorage((void*)&((*inst.*field)()));
    }
    template<typename C, typename T> void*
    fieldStorage_memobj(C* inst, T C::*field)
    {
      return typeOf<T>()->initializeStorage(&((*inst.*field)));
    }
    template<typename C, typename F>
    typename boost::enable_if<boost::is_member_object_pointer<F>,void*>::type
    fieldStorage(C* inst, F field)
    {
      return fieldStorage_memobj(inst, field);
    }

    template<typename C, typename T> void* fieldStorage(C* inst, T& (C::*field)(void) const)
    {
      return typeOf<T>()->initializeStorage((void*)&((*inst.*field)()));
    }
    template<typename C, typename T> void* fieldStorage(C* inst, T& (*field)(C*))
    {
       return typeOf<T>()->initializeStorage((void*)&((*field)(inst)));
    }
    template<typename F> struct FieldType{};
    template<typename C, typename T> struct FieldType<T C::*>
    {
     typedef T type;
    };
    template<typename C, typename F>
    typename boost::enable_if<boost::is_member_object_pointer<F>, typename FieldType<F>::type&>::type
    fieldValue(C* inst, F field, void** data)
    {
      typedef typename  FieldType<F>::type T;
      return *(T*)typeOf<T>()->ptrFromStorage(data);
    }
    template<typename C, typename T> T& fieldValue(C* inst, T& (C::*field)(void), void** data)
    {
      return *(T*)typeOf<T>()->ptrFromStorage(data);
    }
    template<typename C, typename T> T& fieldValue(C* inst, T& (C::*field)(void) const, void** data)
    {
      return *(T*)typeOf<T>()->ptrFromStorage(data);
    }
    template<typename C, typename T> T& fieldValue(C* inst, const T& (*field)(C*), void** data)
    {
      return *(T*)typeOf<T>()->ptrFromStorage(data);
    }
  }
}

#define QI_TYPE_STRUCT_DECLARE(name)                                      \
 __QI_TYPE_STRUCT_DECLARE(name, /**/)

#define __QI_TYPE_STRUCT_DECLARE(name, extra)                              \
namespace qi {                                                            \
  template<> struct TypeImpl<name>: public ::qi::TypeTuple                \
  {                                                                       \
  public:                                                                 \
    typedef name ClassType;                                               \
    virtual std::vector< ::qi::Type*> memberTypes();                      \
    virtual std::vector<std::string> annotations();                       \
    virtual void* get(void* storage, unsigned int index);                 \
    virtual void set(void** storage, unsigned int index, void* valStorage); \
    extra                                                                   \
    _QI_BOUNCE_TYPE_METHODS(::qi::DefaultTypeImplMethods<name>);            \
 }; }


#define __QI_TUPLE_TYPE(_, what, field) res.push_back(::qi::typeOf(ptr->field));
#define __QI_TUPLE_GET(_, what, field) if (i == index) return ::qi::typeOf(ptr->field)->initializeStorage(&ptr->field); i++;
#define __QI_TUPLE_SET(_, what, field) if (i == index) ::qi::detail::setFromStorage(ptr->field, valueStorage); i++;
#define __QI_TUPLE_FIELD_NAME(_, what, field) res.push_back(BOOST_PP_STRINGIZE(QI_DELAY(field)));
#define __QI_TYPE_STRUCT_IMPLEMENT(name, inl, onSet, ...)                                    \
namespace qi {                                                                        \
  inl std::vector< ::qi::Type*> TypeImpl<name>::memberTypes()                                \
  {                                                                                   \
    name* ptr = 0;                                                                    \
    std::vector< ::qi::Type*> res;                                                           \
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
  inl std::vector<std::string> TypeImpl<name>::annotations() \
  {  \
    std::vector<std::string> res; \
    QI_VAARGS_APPLY(__QI_TUPLE_FIELD_NAME, _, __VA_ARGS__); \
    return res; \
  }\
}

/// Declare a struct field using an helper function
#define QI_STRUCT_HELPER(name, func) (name, func, FUNC)
/// Declare a struct feld that is a member (member value or member accessor function)
#define QI_STRUCT_FIELD(name, field) (name, field, FIELD)

// construct pointer-to accessor from free-function
#define __QI_STRUCT_ACCESS_FUNC(fname, field) &field
// construct pointer-to-accessor from member function/field
#define __QI_STRUCT_ACCESS_FIELD(fname, field) &ClassType::field

// invoke the correct __QI_STRUCT_ACCESS_ macro using type
#define __QI_STRUCT_ACCESS_BOUNCE2(name, accessor, type) \
  QI_CAT(__QI_STRUCT_ACCESS_, type)(name, accessor)

// bounce with default value FIELD for argument 3
#define __QI_STRUCT_ACCESS_BOUNCE1(x, y) \
  __QI_STRUCT_ACCESS_BOUNCE2(x, y, FIELD)

// arg-count overload, bounce to __QI_STRUCT_ACCESS_BOUNCE<N>
#define __QI_STRUCT_ACCESS_BOUNCE(...) \
 QI_CAT(__QI_STRUCT_ACCESS_BOUNCE, QI_LIST_VASIZE((__VA_ARGS__)))(__VA_ARGS__)

// accept (name, accessor, type) and (name, accessor) defaulting type to field
#define __QI_STRUCT_ACCESS(tuple) QI_DELAY(__QI_STRUCT_ACCESS_BOUNCE)tuple


#define __QI_ATUPLE_TYPE(_, what, field) res.push_back(::qi::detail::fieldType(__QI_STRUCT_ACCESS(field)));
#define __QI_ATUPLE_GET(_, what, field) if (i == index) return ::qi::detail::fieldStorage(ptr, __QI_STRUCT_ACCESS(field)); i++;
#define __QI_ATUPLE_FIELD_NAME(_, what, field) res.push_back(QI_PAIR_FIRST(field));
#define __QI_ATUPLE_FROMDATA(idx, what, field) ::qi::detail::fieldValue(ptr, __QI_STRUCT_ACCESS(field), &data[idx])
#define __QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_IMPLEMENT(name, inl, onSet, ...)\
namespace qi {                                                                        \
  inl std::vector< ::qi::Type*> TypeImpl<name>::memberTypes()                                \
  {                                                                                   \
    std::vector< ::qi::Type*> res;                                                           \
    QI_VAARGS_APPLY(__QI_ATUPLE_TYPE, name, __VA_ARGS__);                                 \
    return res;                                                                       \
  }                                                                                   \
  inl void* TypeImpl<name>::get(void* storage, unsigned int index)                    \
  {                                                                                   \
    unsigned int i = 0;                                                                        \
    name* ptr = (name*)ptrFromStorage(&storage);                                      \
    QI_VAARGS_APPLY(__QI_ATUPLE_GET, name, __VA_ARGS__);                                  \
    return 0;                                                                         \
  }                                                                                   \
  inl void TypeImpl<name>::set(void** storage, unsigned int index, void* valueStorage)\
  {\
    throw std::runtime_error("single-field set not implemented");\
  }\
  inl void TypeImpl<name>::set(void** storage, std::vector<void*> data) \
  {\
     name* ptr = (name*)ptrFromStorage(storage);         \
     *ptr = name(QI_VAARGS_MAP(__QI_ATUPLE_FROMDATA, name, __VA_ARGS__)); \
  }\
  inl std::vector<std::string> TypeImpl<name>::annotations() \
  {  \
    std::vector<std::string> res; \
    QI_VAARGS_APPLY(__QI_ATUPLE_FIELD_NAME, _, __VA_ARGS__); \
    return res; \
  }\
}


/// Allow the QI_TYPE_STRUCT macro and variants to access private members
#define QI_TYPE_STRUCT_PRIVATE_ACCESS(name) \
friend class qi::TypeImpl<name>;

/** Declare a simple struct to the type system.
 * First argument is the structure name. Remaining arguments are the structure
 * fields.
 * This macro must be called outside any namespace.
 * This macro should be called in the header file defining the structure 'name',
 * or in a header included by all source files using the structure.
 * See QI_TYPE_STRUCT_REGISTER for a similar macro that can be called from a
 * single source file.
 */
#define QI_TYPE_STRUCT(name, ...) \
  QI_TYPE_STRUCT_DECLARE(name) \
  __QI_TYPE_STRUCT_IMPLEMENT(name, inline, /**/, __VA_ARGS__)

/** Similar to QI_TYPE_STRUCT, but evaluates 'onSet' after writting to an instance.
 * The instance is accessible through the variable 'ptr'.
 */
#define QI_TYPE_STRUCT_EX(name, onSet, ...) \
  QI_TYPE_STRUCT_DECLARE(name) \
  __QI_TYPE_STRUCT_IMPLEMENT(name, inline, onSet, __VA_ARGS__)

#define QI_TYPE_STRUCT_IMPLEMENT(name, ...) \
  __QI_TYPE_STRUCT_IMPLEMENT(name, /**/, /**/, __VA_ARGS__)

/** Register a struct with member field/function getters, and constructor setter
 *
 * Call like that:
 *    QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(Point,
 *       ("x", &Point::getX),
 *       ("y", &Point::y))
 *
 * The first macro argument is the class full name including namespace.
 * Remaining arguments can be:
 *  - (fieldName, accessor): accessor must be:
 *        - a member function returning a const T& with no argument
 *        - a member field
 *  - QI_STRUCT_FIELD(fieldName, accessor): Same thing as above, provided for
 *      consistency.
 *  - QI_STRUCT_HELPER(fieldName, function). Function must be a free function
 *       taking a class pointer, and returning a const T&
 *
 * The class must provide a constructor that accepts all fields as argument, in
 * the order in which they are declared in the macro.
 */
#define QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(name, ...)     \
  __QI_TYPE_STRUCT_DECLARE(name,                             \
    virtual void set(void** storage, std::vector<void*>);) \
    __QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_IMPLEMENT(name, inline, /**/, __VA_ARGS__)

/** Similar to QI_TYPE_STRUCT, but using the runtime factory instead of the
 * compile-time template.
 *
 */
#define QI_TYPE_STRUCT_REGISTER(name, ...) \
namespace _qi_ {                           \
    QI_TYPE_STRUCT(name, __VA_ARGS__);     \
}                                          \
QI_TYPE_REGISTER_CUSTOM(name, _qi_::qi::TypeImpl<name>)

/** Similar to QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR,
 * but using the runtime factory instead of the
 * compile-time template.
 *
 */
#define QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_REGISTER(name, ...) \
namespace _qi_ {                           \
    QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(name, __VA_ARGS__);     \
}                                          \
QI_TYPE_REGISTER_CUSTOM(name, _qi_::qi::TypeImpl<name>)

/** Declares that name is equivalent to type bounceTo, and that instances
 * can be converted using the conversion function with signature 'bounceTo* (name*)'.
 * This macro should be called in a header included by all code using the 'name'
 * class.
 * See QI_TYPE_STRUCT_BOUNCE_REGISTER for a similar macro that can be called from a
 * single source file.
 */
#define QI_TYPE_STRUCT_BOUNCE(name, bounceTo, conversion)                 \
namespace qi {                                                            \
template<> class TypeImpl<name>: public ::qi::TypeTupleBouncer<name, bounceTo>  \
{                                                                         \
public:                                                                   \
  void adaptStorage(void** storage, void** adapted)                       \
  {                                                                       \
    name* ptr = (name*)ptrFromStorage(storage);                           \
    bounceTo * tptr = conversion(ptr);                                    \
    *adapted = bounceType()->initializeStorage(tptr);                     \
  }                                                                       \
};}

/** Similar to QI_TYPE_STRUCT_BOUNCE, but using the runtime factory instead of the
 * compile-time template.
 */
#define QI_TYPE_STRUCT_BOUNCE_REGISTER(name, bounceTo, conversion)        \
namespace _qi_ {                                                          \
    QI_TYPE_STRUCT_BOUNCE(name, bounceTo, conversion);                    \
}                                                                         \
QI_TYPE_REGISTER_CUSTOM(name, _qi_::qi::TypeImpl<name>)



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
    virtual std::vector<Type*> memberTypes()
    {
      return bounceType()->memberTypes();
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

  template<typename F, typename S>
  class TypeImpl<std::pair<F, S> >: public TypeTuple
  {
  public:
    typedef DefaultTypeImplMethods<std::pair<F, S> > Methods;
    typedef typename std::pair<F, S> BackendType;
    std::vector<Type*> memberTypes()
    {
      static std::vector<Type*>* result=0;
      if (!result)
      {
        result = new std::vector<Type*>();
        result->push_back(typeOf<F>());
        result->push_back(typeOf<S>());
      }
      return *result;
    }
    void* get(void* storage, unsigned int index)
    {
      BackendType* ptr = (BackendType*)ptrFromStorage(&storage);
      // Will work if F or S are references
      if (!index)
        return typeOf<F>()->initializeStorage(const_cast<void*>((void*)&ptr->first));
      else
        return typeOf<S>()->initializeStorage(const_cast<void*>((void*)&ptr->second));
    }
    void set(void** storage, unsigned int index, void* valStorage)
    {
      BackendType* ptr = (BackendType*)ptrFromStorage(storage);
      if (!index)
        detail::TypeManagerDefault<F>::copy(const_cast<void*>((void*)&ptr->first), typeOf<F>()->ptrFromStorage(&valStorage));
      else
        detail::TypeManagerDefault<S>::copy(const_cast<void*>((void*)&ptr->second), typeOf<S>()->ptrFromStorage(&valStorage));
    }
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

}
#endif  // _QITYPE_DETAILS_TYPETUPLE_HXX_

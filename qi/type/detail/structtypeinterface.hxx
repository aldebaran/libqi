#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_TYPETUPLE_HXX_
#define _QITYPE_DETAIL_TYPETUPLE_HXX_

#include <map>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <qi/api.hpp>
#include <qi/type/fwd.hpp>
#include <qi/type/detail/accessor.hxx>
#include <qi/type/typeinterface.hpp>
#include <qi/preproc.hpp>

namespace qi
{
  namespace detail {

    bool QI_API fillMissingFieldsWithDefaultValues(
        std::map<std::string, ::qi::AnyValue>& fields,
        const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
        const char** which=0, int whichLength=0);

    template <typename T>
    struct StructVersioningDelegateAddFields
    {
      static bool convertFrom(StructTypeInterface* /*type*/,
                              std::map<std::string, qi::AnyValue>& /*fields*/,
                              const std::vector<std::tuple<std::string, TypeInterface*>>& missing)
      {
        return missing.empty();
      }
      static bool convertTo(const std::map<std::string, qi::AnyReference>& dropFields)
      {
        return dropFields.empty();
      }
    };
    template <typename T>
    struct StructVersioningDelegateDropFields
    {
      static bool convertTo(StructTypeInterface* /*type*/,
                            std::map<std::string, ::qi::AnyValue>& /*fields*/,
                            const std::vector<std::tuple<std::string, TypeInterface*>>& missing)
      {
        return missing.empty();
      }
      static bool convertFrom(const std::map<std::string, qi::AnyReference>& dropFields)
      {
        return dropFields.empty();
      }
    };

    template <typename T>
    struct StructVersioningDelegate
    {
      static bool convertFrom(StructTypeInterface* type,
                              std::map<std::string, qi::AnyValue>& fields,
                              const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
                              const std::map<std::string, qi::AnyReference>& dropfields)
      {
        return StructVersioningDelegateDropFields<T>::convertFrom(dropfields) &&
               StructVersioningDelegateAddFields<T>::convertFrom(type, fields, missing);
      }
      static bool convertTo(StructTypeInterface* type,
                            std::map<std::string, ::qi::AnyValue>& fields,
                            const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
                            const std::map<std::string, ::qi::AnyReference>& dropfields)
      {
        return StructVersioningDelegateAddFields<T>::convertTo(dropfields) &&
               StructVersioningDelegateDropFields<T>::convertTo(type, fields, missing);
      }
    };

/** Declare that some fields have been added in this structure. They will be filled with default values upon conversion
 * to this structure.
 */
#define QI_TYPE_STRUCT_EXTENSION_ADDED_FIELDS(name, ...)                                             \
  namespace qi                                                                                       \
  {                                                                                                  \
    namespace detail                                                                                 \
    {                                                                                                \
      template <>                                                                                    \
      struct StructVersioningDelegateAddFields<name>                                                 \
      {                                                                                              \
        static bool convertFrom(StructTypeInterface* /*type*/,                                       \
                                std::map<std::string, ::qi::AnyValue>& fields,                       \
                                const std::vector<std::tuple<std::string, TypeInterface*>>& missing) \
        {                                                                                            \
          static const char* which[] = {__VA_ARGS__};                                                \
          const int count = sizeof(which) / sizeof(char*);                                           \
          return fillMissingFieldsWithDefaultValues(fields, missing, which, count);            \
        }                                                                                            \
        static bool convertTo(const std::map<std::string, ::qi::AnyReference>& todrop)               \
        {                                                                                            \
          static const char* which[] = {__VA_ARGS__};                                                \
          const int count = sizeof(which) / sizeof(char*);                                           \
          for (const auto& field : todrop)                                                           \
            if (std::find(which, which + count, field.first) == which + count)                       \
              return false;                                                                          \
          return true;                                                                               \
        }                                                                                            \
      };                                                                                             \
    }                                                                                                \
  }
/** Declare that some fields have been dropped in this structure. They will be filled with default values upon
 * conversion to the old structure.
 */
#define QI_TYPE_STRUCT_EXTENSION_DROPPED_FIELDS(name, ...)                                         \
  namespace qi                                                                                     \
  {                                                                                                \
    namespace detail                                                                               \
    {                                                                                              \
      template <>                                                                                  \
      struct StructVersioningDelegateDropFields<name>                                              \
      {                                                                                            \
        static bool convertTo(StructTypeInterface* /*type*/,                                       \
                              std::map<std::string, ::qi::AnyValue>& fields,                       \
                              const std::vector<std::tuple<std::string, TypeInterface*>>& missing) \
        {                                                                                          \
          static const char* which[] = {__VA_ARGS__};                                              \
          const int count = sizeof(which) / sizeof(char*);                                         \
          return fillMissingFieldsWithDefaultValues(fields, missing, which, count);          \
        }                                                                                          \
        static bool convertFrom(const std::map<std::string, ::qi::AnyReference>& todrop)           \
        {                                                                                          \
          static const char* which[] = {__VA_ARGS__};                                              \
          const int count = sizeof(which) / sizeof(char*);                                         \
          for (const auto& field : todrop)                                                         \
            if (std::find(which, which + count, field.first) == which + count)                     \
              return false;                                                                        \
          return true;                                                                             \
        }                                                                                          \
      };                                                                                           \
    }                                                                                              \
  }

/** Declare handlers for structure conversion from and to old version.
 *
 * \param name the name of the structure
 * \param fromHandler a handler that will be called when converting from old structure to this one. It should have the
 * following signature: bool convertFrom(StructTypeInterface* type, std::map<std::string, ::qi::AnyValue>& fields, const
 * std::vector<std::tuple<std::string, TypeInterface*>>& missing, const std::map<std::string, ::qi::AnyReference>&
 * dropfields). The method receives the structure to convert from as a map and it must fill in the fields given in the
 * missing argument. It must return true upon successful conversion.
 * \param toHandler a handler that will be called when converting from this structure to the old one. It should have the
 * following signature: bool convertFrom(StructTypeInterface* type, std::map<std::string, ::qi::AnyValue>& fields, const
 * std::vector<std::tuple<std::string>& missing, const std::map<std::string, ::qi::AnyReference>& dropfields). The
 * method receives the structure to convert from as a map and it must fill in the fields given in the missing argument.
 * It must return true upon successful conversion.
 */
#define QI_TYPE_STRUCT_EXTENSION_CONVERT_HANDLERS(name, fromHandler, toHandler)                      \
  namespace qi                                                                                       \
  {                                                                                                  \
    namespace detail                                                                                 \
    {                                                                                                \
      template <>                                                                                    \
      struct StructVersioningDelegate<name>                                                          \
      {                                                                                              \
        static bool convertFrom(StructTypeInterface* /*type*/,                                       \
                                std::map<std::string, ::qi::AnyValue>& fields,                       \
                                const std::vector<std::tuple<std::string, TypeInterface*>>& missing, \
                                const std::map<std::string, ::qi::AnyReference>& dropfields)         \
        {                                                                                            \
          return fromHandler(fields, missing, dropfields);                                           \
        }                                                                                            \
        static bool convertTo(StructTypeInterface* /*type*/,                                         \
                              std::map<std::string, ::qi::AnyValue>& fields,                         \
                              const std::vector<std::tuple<std::string, TypeInterface*>>& missing,   \
                              const std::map<std::string, ::qi::AnyReference>& dropfields)           \
        {                                                                                            \
          return toHandler(fields, missing, dropfields);                                             \
        }                                                                                            \
      };                                                                                             \
    }                                                                                                \
  }

    //keep only the class name. (remove :: and namespaces)
    QI_API std::string normalizeClassName(const std::string &name);

    template<typename T> void setFromStorage(T& ref, void* storage)
    {
      ref = *(T*)typeOf<T>()->ptrFromStorage(&storage);
    }

    /* Helpers around accessors
     */
    template<typename A>
    TypeInterface* fieldType(A)
    {
      static TypeInterface* const res = qi::typeOf<detail::accessor::ValueType<A>>();
      return res;
    }

    template<typename C, typename A>
    void* fieldStorage(C* inst, A accessor)
    {
      auto& value = detail::accessor::invoke(accessor, inst);
      return fieldType(accessor)->initializeStorage(const_cast<void*>(static_cast<const void*>(&value)));
    }

    template<typename C, typename A>
    detail::accessor::ValueType<A>& fieldValue(C* /*instance*/, A accessor, void** data)
    {
      using T = detail::accessor::ValueType<A>;
      return *reinterpret_cast<T*>(fieldType(accessor)->ptrFromStorage(data));
    }
  }
}

#define __QI_TYPE_STRUCT_DECLARE(name, extra)                                                         \
  namespace qi                                                                                        \
  {                                                                                                   \
    template <>                                                                                       \
    struct TypeImpl<name> : public ::qi::StructTypeInterface                                          \
    {                                                                                                 \
    public:                                                                                           \
      using ClassType = name;                                                                         \
      TypeImpl();                                                                                     \
      std::vector<::qi::TypeInterface*> memberTypes() override;                                       \
      std::vector<std::string> elementsName() override;                                               \
      std::string className() override;                                                               \
      void* get(void* storage, unsigned int index) override;                                          \
      void set(void** storage, unsigned int index, void* valStorage) override;                        \
      virtual bool convertFrom(std::map<std::string, ::qi::AnyValue>& fields,                         \
                               const std::vector<std::tuple<std::string, TypeInterface*>>& missing,   \
                               const std::map<std::string, ::qi::AnyReference>& dropfields) override; \
      virtual bool convertTo(std::map<std::string, ::qi::AnyValue>& fields,                           \
                             const std::vector<std::tuple<std::string, TypeInterface*>>& missing,     \
                             const std::map<std::string, ::qi::AnyReference>& dropfields) override;   \
      extra using Impl = ::qi::DefaultTypeImplMethods<name, ::qi::TypeByPointerPOD<name>>;            \
      _QI_BOUNCE_TYPE_METHODS(Impl);                                                                  \
    };                                                                                                \
  }

#define __QI_TUPLE_TYPE(_, what, field) res.push_back(::qi::typeOf(ptr->field));
#define __QI_TUPLE_GET(_, what, field) if (i == index) return ::qi::typeOf(ptr->field)->initializeStorage(&ptr->field); i++;
#define __QI_TUPLE_SET(_, what, field) if (i == index) ::qi::detail::setFromStorage(ptr->field, valueStorage); i++;
#define __QI_TUPLE_FIELD_NAME(_, what, field) res.push_back(BOOST_PP_STRINGIZE(QI_DELAY(field)));
#define __QI_TYPE_STRUCT_IMPLEMENT(name, inl, onSet, ...)                                                     \
  namespace qi                                                                                                \
  {                                                                                                           \
    inl TypeImpl<name>::TypeImpl()                                                                            \
    {                                                                                                         \
      ::qi::registerStruct(this);                                                                             \
    }                                                                                                         \
    inl std::vector<::qi::TypeInterface*> TypeImpl<name>::memberTypes()                                       \
    {                                                                                                         \
      name* ptr = 0;                                                                                          \
      std::vector<::qi::TypeInterface*> res;                                                                  \
      QI_VAARGS_APPLY(__QI_TUPLE_TYPE, _, __VA_ARGS__);                                                       \
      return res;                                                                                             \
    }                                                                                                         \
    inl void* TypeImpl<name>::get(void* storage, unsigned int index)                                          \
    {                                                                                                         \
      unsigned int i = 0;                                                                                     \
      name* ptr = (name*)ptrFromStorage(&storage);                                                            \
      QI_VAARGS_APPLY(__QI_TUPLE_GET, _, __VA_ARGS__);                                                        \
      return 0;                                                                                               \
    }                                                                                                         \
    inl void TypeImpl<name>::set(void** storage, unsigned int index, void* valueStorage)                      \
    {                                                                                                         \
      unsigned int i = 0;                                                                                     \
      name* ptr = (name*)ptrFromStorage(storage);                                                             \
      QI_VAARGS_APPLY(__QI_TUPLE_SET, _, __VA_ARGS__);                                                        \
      onSet                                                                                                   \
    }                                                                                                         \
    inl std::vector<std::string> TypeImpl<name>::elementsName()                                               \
    {                                                                                                         \
      std::vector<std::string> res;                                                                           \
      QI_VAARGS_APPLY(__QI_TUPLE_FIELD_NAME, _, __VA_ARGS__);                                                 \
      return res;                                                                                             \
    }                                                                                                         \
    inl std::string TypeImpl<name>::className()                                                               \
    {                                                                                                         \
      return ::qi::detail::normalizeClassName(BOOST_PP_STRINGIZE(name));                                      \
    }                                                                                                         \
    inl bool TypeImpl<name>::convertFrom(std::map<std::string, ::qi::AnyValue>& fields,                       \
                                         const std::vector<std::tuple<std::string, TypeInterface*>>& missing, \
                                         const std::map<std::string, ::qi::AnyReference>& dropfields)         \
    {                                                                                                         \
      return ::qi::detail::StructVersioningDelegate<name>::convertFrom(this, fields, missing, dropfields);    \
    }                                                                                                         \
    inl bool TypeImpl<name>::convertTo(std::map<std::string, ::qi::AnyValue>& fields,                         \
                                       const std::vector<std::tuple<std::string, TypeInterface*>>& missing,   \
                                       const std::map<std::string, ::qi::AnyReference>& dropfields)           \
    {                                                                                                         \
      return ::qi::detail::StructVersioningDelegate<name>::convertTo(this, fields, missing, dropfields);      \
    }                                                                                                         \
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
#define __QI_ATUPLE_FROMDATA(idx, what, field) ::qi::detail::fieldValue(ptr, __QI_STRUCT_ACCESS(field), const_cast<void**>(&data[idx]))
#define __QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_IMPLEMENT(name, inl, onSet, ...)                                \
  namespace qi                                                                                                \
  {                                                                                                           \
    inl TypeImpl<name>::TypeImpl()                                                                            \
    {                                                                                                         \
      ::qi::registerStruct(this);                                                                             \
    }                                                                                                         \
    inl std::vector<::qi::TypeInterface*> TypeImpl<name>::memberTypes()                                       \
    {                                                                                                         \
      std::vector<::qi::TypeInterface*> res;                                                                  \
      QI_VAARGS_APPLY(__QI_ATUPLE_TYPE, name, __VA_ARGS__);                                                   \
      return res;                                                                                             \
    }                                                                                                         \
                                                                                                              \
    inl void* TypeImpl<name>::get(void* storage, unsigned int index)                                          \
    {                                                                                                         \
      unsigned int i = 0;                                                                                     \
      name* ptr = (name*)ptrFromStorage(&storage);                                                            \
      QI_VAARGS_APPLY(__QI_ATUPLE_GET, name, __VA_ARGS__);                                                    \
      return 0;                                                                                               \
    }                                                                                                         \
                                                                                                              \
    inl void TypeImpl<name>::set(void** /*storage*/, unsigned int /*index*/, void* /*valueStorage*/)          \
    {                                                                                                         \
      throw std::runtime_error("single-field set not implemented");                                           \
    }                                                                                                         \
                                                                                                              \
    inl void TypeImpl<name>::set(void** storage, const std::vector<void*>& data)                              \
    {                                                                                                         \
      name* ptr = (name*)ptrFromStorage(storage);                                                             \
      *ptr = name(QI_VAARGS_MAP(__QI_ATUPLE_FROMDATA, name, __VA_ARGS__));                                    \
    }                                                                                                         \
                                                                                                              \
    inl std::vector<std::string> TypeImpl<name>::elementsName()                                               \
    {                                                                                                         \
      std::vector<std::string> res;                                                                           \
      QI_VAARGS_APPLY(__QI_ATUPLE_FIELD_NAME, _, __VA_ARGS__);                                                \
      return res;                                                                                             \
    }                                                                                                         \
    inl std::string TypeImpl<name>::className()                                                               \
                                                                                                              \
    {                                                                                                         \
      return ::qi::detail::normalizeClassName(BOOST_PP_STRINGIZE(name));                                      \
    }                                                                                                         \
    inl bool TypeImpl<name>::convertFrom(std::map<std::string, ::qi::AnyValue>& /*fields*/,                       \
                                         const std::vector<std::tuple<std::string, TypeInterface*>>& /*missing*/, \
                                         const std::map<std::string, ::qi::AnyReference>& /*dropfields*/)         \
    {                                                                                                             \
      return false;                                                                                               \
    }                                                                                                             \
    inl bool TypeImpl<name>::convertTo(std::map<std::string, ::qi::AnyValue>& /*fields*/,                         \
                                       const std::vector<std::tuple<std::string, TypeInterface*>>& /*missing*/,   \
                                       const std::map<std::string, ::qi::AnyReference>& /*dropfields*/)           \
    {                                                                                                             \
      return false;                                                                                               \
    }                                                                                                             \
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
 *       ("x", getX),
 *       ("y", y))
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
 *
 * Must be called outside any namespace.
 */
#define QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(name, ...)     \
  __QI_TYPE_STRUCT_DECLARE(name,                             \
    void set(void** storage, const std::vector<void*>&) override;) \
    __QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_IMPLEMENT(name, inline, /**/, __VA_ARGS__)

/** Similar to QI_TYPE_STRUCT, but using the runtime factory instead of the
 * compile-time template. This macro will register the struct at static
 * initialization time, and thus should only be called from one compilation
 * unit. To ensure this, the simplest option is to use this macro from a .cpp
 * source file. It should *not* be used in a header.
 */
#define QI_TYPE_STRUCT_REGISTER(name, ...) \
namespace _qi_ {                           \
    QI_TYPE_STRUCT(name, __VA_ARGS__)      \
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
template<> class TypeImpl<name>: public ::qi::StructTypeInterfaceBouncer<name, bounceTo>  \
{                                                                         \
public:                                                                   \
  void adaptStorage(void** storage, void** adapted)                       \
  {                                                                       \
    name* ptr = (name*)ptrFromStorage(storage);                           \
    bounceTo * tptr = conversion(ptr);                                    \
    *adapted = bounceType()->initializeStorage(tptr);                     \
  }                                                                       \
  std::string className()                                                 \
  {                                                                       \
    return ::qi::detail::normalizeClassName(BOOST_PP_STRINGIZE(name));    \
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
  template<typename T, typename TO>
  class StructTypeInterfaceBouncer: public StructTypeInterface
  {
  public:
    StructTypeInterface* bounceType()
    {
      static TypeInterface* result = 0;
      if (!result)
        result = typeOf<TO>();
      return static_cast<StructTypeInterface*>(result);
    }

    virtual void adaptStorage(void** storage, void** adapted) = 0;

    using Methods = DefaultTypeImplMethods<T, TypeByPointerPOD<T>>;
    std::vector<TypeInterface*> memberTypes() override
    {
      return bounceType()->memberTypes();
    }

    void* get(void* storage, unsigned int index) override
    {
      void* astorage;
      adaptStorage(&storage, &astorage);
      return bounceType()->get(astorage, index);
    }

    std::vector<void*> get(void* storage) override
    {
      void* astorage;
      adaptStorage(&storage, &astorage);
      return bounceType()->get(astorage);
    }

    void set(void** storage, const std::vector<void*>& vals) override
    {
      void* astorage;
      adaptStorage(storage, &astorage);
      bounceType()->set(&astorage, vals);
    }

    void set(void** storage, unsigned int index, void* valStorage) override
    {
      void* astorage;
      adaptStorage(storage, &astorage);
      bounceType()->set(&astorage, index, valStorage);
    }

    std::vector<std::string> elementsName() override
    {
      return bounceType()->elementsName();
    }

    virtual bool convertFrom(std::map<std::string, qi::AnyValue>& fields,
                             const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
                             const std::map<std::string, qi::AnyReference>& dropfields) override
    {
      return bounceType()->convertFrom(fields, missing, dropfields);
    }

    virtual bool convertTo(std::map<std::string, qi::AnyValue>& fields,
                           const std::vector<std::tuple<std::string, TypeInterface*>>& missing,
                           const std::map<std::string, qi::AnyReference>& dropfields) override
    {
      return bounceType()->convertTo(fields, missing, dropfields);
    }

    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  template<typename F, typename S>
  class TypeImpl<std::pair<F, S> >: public StructTypeInterface
  {
  public:
    using Methods = DefaultTypeImplMethods<std::pair<F, S>, TypeByPointerPOD<std::pair<F,S>>>;
    using BackendType = typename std::pair<F, S>;
    TypeImpl()
    {
      _memberTypes.push_back(typeOf<F>());
      _memberTypes.push_back(typeOf<S>());
    }
    std::vector<TypeInterface*> _memberTypes;

    std::vector<TypeInterface*> memberTypes() override { return _memberTypes;}
    void* get(void* storage, unsigned int index) override
    {
      BackendType* ptr = (BackendType*)ptrFromStorage(&storage);
      // Will work if F or S are references
      if (!index)
        return typeOf<F>()->initializeStorage(const_cast<void*>((void*)&ptr->first));
      else
        return typeOf<S>()->initializeStorage(const_cast<void*>((void*)&ptr->second));
    }
    void set(void** storage, unsigned int index, void* valStorage) override
    {
      BackendType* ptr = (BackendType*)ptrFromStorage(storage);
      const std::vector<TypeInterface*>& types = _memberTypes;


      // FIXME cheating, we do not go through TypeInterface of S and F for copy
      // because typeerasure does not expose the interface
      if (!index)
        detail::TypeTraitCopy<typename boost::remove_const<F>::type, true>::copy(const_cast<void*>((void*)&ptr->first), types[0]->ptrFromStorage(&valStorage));
      else
        detail::TypeTraitCopy<typename boost::remove_const<S>::type, true>::copy(const_cast<void*>((void*)&ptr->second), types[1]->ptrFromStorage(&valStorage));
    }
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

}
#endif  // _QITYPE_DETAIL_TYPETUPLE_HXX_

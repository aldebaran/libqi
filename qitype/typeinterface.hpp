#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_TYPEINTERFACE_HPP_
#define _QITYPE_TYPEINTERFACE_HPP_

#include <typeinfo>
#include <string>

#include <boost/preprocessor.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/mpl/if.hpp>

#include <qi/log.hpp>
#include <qitype/api.hpp>
#include <qitype/fwd.hpp>
#include <qitype/signature.hpp>
#include <qitype/details/typeinterface.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
   // C4503 decorated name length exceeded, name was truncated
   // The only workaround is to make structs to hide the template complexity
   // We don't want to have to do that
#  pragma warning( disable: 4503 )
#endif

/* A lot of class are found in this headers... to kill circular dependencies.
   Futhermore we need that all "default template" types are registered (included)
   when type.hpp is used. (for typeOf to works reliably)
*/

namespace qi{


    /// Declare that a type has no accessible default constructor.
    /// \warning Be careful to put the declaration outside any namespaces.
  #define QI_TYPE_NOT_CONSTRUCTIBLE(T) \
    namespace qi { namespace detail {  \
    template<> struct TypeManager<T>: public TypeManagerNonDefaultConstructible<T> {};}}

    /// Declare that a type has no metatype and cannot be used in a Value
    /// \warning Be careful to put the declaration outside any namespaces.
  #define QI_NO_TYPE(T) namespace qi {template<> class TypeImpl<T>: public detail::ForbiddenInTypeSystem {};}

    /// Declare that a type has no accessible copy constructor
    /// \warning Be careful to put the declaration outside any namespaces.
  #define QI_TYPE_NOT_CLONABLE(T)     \
    namespace qi { namespace detail { \
    template<> struct TypeManager<T>: public TypeManagerNull<T> {};}}

    /// Register TypeImpl<t> in runtime type factory for 't'. Must be called from a .cpp file
    /// \warning Be careful to put the declaration outside any namespaces.
  #define QI_TYPE_REGISTER(t) \
    QI_TYPE_REGISTER_CUSTOM(t, qi::TypeImpl<t>)

    /// Register 'typeimpl' in runtime type factory for 'type'.
    /// \warning Be careful to put the declaration outside any namespaces.
  #define QI_TYPE_REGISTER_CUSTOM(type, typeimpl) \
    static bool BOOST_PP_CAT(__qi_registration, __LINE__) = qi::registerType(typeid(type), new typeimpl)


  class ListTypeInterface;
  class StructTypeInterface;

  // Interfaces for specialized types
  class QITYPE_API IntTypeInterface: public TypeInterface
  {
  public:
    virtual int64_t get(void* value) const = 0;
    virtual unsigned int size() const = 0; // size in bytes
    virtual bool isSigned() const = 0; // return if type is signed
    virtual void set(void** storage, int64_t value) = 0;
    virtual TypeKind kind() const { return TypeKind_Int;}
  };

  class QITYPE_API FloatTypeInterface: public TypeInterface
  {
  public:
    virtual double get(void* value) const = 0;
    virtual unsigned int size() const = 0; // size in bytes
    virtual void set(void** storage, double value) = 0;
    virtual TypeKind kind() const { return TypeKind_Float;}
  };

  class Buffer;
  class QITYPE_API StringTypeInterface: public TypeInterface
  {
  public:
    std::string getString(void* storage) const;
    virtual std::pair<char*, size_t> get(void* storage) const = 0;
    void set(void** storage, const std::string& value);
    virtual void set(void** storage, const char* ptr, size_t sz) = 0;
    virtual TypeKind kind() const { return TypeKind_String; }

  };

  class QITYPE_API RawTypeInterface: public TypeInterface
  {
  public:
    virtual std::pair<char*, size_t> get(void* storage) const = 0;
    virtual void set(void** storage, const char* ptr, size_t sz) = 0;
    virtual TypeKind kind() const { return TypeKind_Raw; }
  };

  class QITYPE_API PointerTypeInterface: public TypeInterface
  {
  public:
    enum PointerKind
    {
      Raw,
      Shared,
    };
    virtual PointerKind pointerKind() const = 0;
    virtual TypeInterface* pointedType() const = 0;
    virtual AnyReference dereference(void* storage) = 0; // must not be destroyed
    // Set new pointee value. pointer must be a *pointer* to type pointedType()
    virtual void setPointee(void** storage, void* pointer) = 0;
    virtual TypeKind kind() const { return TypeKind_Pointer; }
  };

  class QITYPE_API IteratorTypeInterface: public TypeInterface
  {
  public:
    // Returned reference is expected to point to somewhere in the iterator, or the container
    virtual AnyReference dereference(void* storage) = 0;
    virtual void  next(void** storage) = 0;
    virtual bool equals(void* s1, void* s2) = 0;
    virtual TypeKind kind() const { return TypeKind_Iterator;}
  };

  class QITYPE_API ListTypeInterface: public TypeInterface
  {
  public:
    virtual TypeInterface* elementType() const = 0;
    virtual size_t size(void* storage) = 0;
    virtual AnyIterator begin(void* storage) = 0;
    virtual AnyIterator end(void* storage) = 0;
    virtual void pushBack(void** storage, void* valueStorage) = 0;
    virtual void* element(void* storage, int index);
    virtual TypeKind kind() const { return TypeKind_List;}
  };

  class QITYPE_API MapTypeInterface: public TypeInterface
  {
  public:
    virtual TypeInterface* elementType() const = 0;
    virtual TypeInterface* keyType() const = 0;
    virtual size_t size(void* storage) = 0;
    virtual AnyIterator begin(void* storage) = 0;
    virtual AnyIterator end(void* storage) = 0;
    virtual void insert(void** storage, void* keyStorage, void* valueStorage) = 0;
    virtual AnyReference element(void** storage, void* keyStorage, bool autoInsert) = 0;
    virtual TypeKind kind() const { return TypeKind_Map; }
    // Since our typesystem has no erased operator < or operator ==,
    // MapTypeInterface does not provide a find()
  };

  class QITYPE_API StructTypeInterface: public TypeInterface
  {
  public:
    std::vector<AnyReference> values(void* storage);
    virtual std::vector<TypeInterface*> memberTypes() = 0;
    virtual std::vector<void*> get(void* storage); // must not be destroyed
    virtual void* get(void* storage, unsigned int index) = 0; // must not be destroyed
    virtual void set(void** storage, std::vector<void*>);
    virtual void set(void** storage, unsigned int index, void* valStorage) = 0; // will copy
    virtual TypeKind kind() const { return TypeKind_Tuple; }
    virtual std::vector<std::string> elementsName() { return std::vector<std::string>();}
    virtual std::string className() { return std::string(); }
  };

  class QITYPE_API DynamicTypeInterface: public TypeInterface
  {
  public:
    virtual AnyReference get(void* storage) = 0;
    virtual void set(void** storage, AnyReference source) = 0;
    virtual TypeKind kind() const { return TypeKind_Dynamic; }
  };

  ///@return a Type of kind List that can contains elements of type elementType.
  QITYPE_API TypeInterface* makeListType(TypeInterface* elementType);

  ///@return a Type of kind Map with given key and element types
  QITYPE_API TypeInterface* makeMapType(TypeInterface* keyType, TypeInterface* ElementType);

  ///@return a Type of kind Tuple with givent memberTypes
  QITYPE_API TypeInterface* makeTupleType(const std::vector<TypeInterface*>& memberTypes, const std::string &name = std::string(), const std::vector<std::string>& elementNames = std::vector<std::string>());



  /** Declare a templated-type taking one type argument.
  * Required to be able to use QI_TEMPLATE_TYPE_GET
  */
  #define QI_TEMPLATE_TYPE_DECLARE(n) \
  namespace qi {              \
    template<typename T> class QITYPE_TEMPLATE_API TypeImpl<n<T> >: public TypeOfTemplateImpl<n, T> {}; \
  }
  /** Return a TemplateTypeInterface pointer if \p typeInst represents an instanciation
   * of template type templateName, 0 otherwise
   */
  #define QI_TEMPLATE_TYPE_GET(typeInst, templateName) \
   dynamic_cast< ::qi::TypeOfTemplate<templateName>*>(typeInst)


  #define QI_TYPE_ENUM_REGISTER(Enum)                                \
    namespace qi {                                                   \
      template<> class TypeImpl<Enum>: public IntTypeInterfaceImpl<long> {};  \
    }

#define QI_TYPE_STRUCT_DECLARE(name)                                      \
 __QI_TYPE_STRUCT_DECLARE(name, /**/)

}


#include <qitype/details/typeimpl.hxx>
#include <qitype/details/type.hxx>
#include <qitype/details/inttypeinterface.hxx>
#include <qitype/details/listtypeinterface.hxx>
#include <qitype/details/maptypeinterface.hxx>
#include <qitype/details/stringtypeinterface.hxx>
#include <qitype/details/pointertypeinterface.hxx>
#include <qitype/details/structtypeinterface.hxx>
#include <qitype/details/buffertypeinterface.hxx>
#include <qitype/details/dynamictypeinterface.hxx>

QI_NO_TYPE(qi::TypeInterface)
QI_NO_TYPE(qi::TypeInterface*)

#ifdef _MSC_VER
#  pragma warning( pop )
// restore the disabling of this warning
#  pragma warning( disable: 4503 )
#endif

#endif  // _QITYPE_TYPEINTERFACE_HPP_

#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPEINTERFACE_HPP_
#define _QITYPE_DETAILS_TYPEINTERFACE_HPP_

#include <typeinfo>
#include <string>
#include <qitype/api.hpp>
#include <qitype/signature.hpp>

/* A lot of class are found in this headers... to kill circular dependencies.
   Futhermore we need that all "default template" types are registered (included)
   when type.hpp is used. (for typeOf to works reliably)
*/

namespace qi {


  /** This class is used to uniquely identify a type.
   *
   */
  class QITYPE_API TypeInfo
  {
  public:
    TypeInfo();
    /// Construct a TypeInfo from a std::type_info
    TypeInfo(const std::type_info& info);
    /// Contruct a TypeInfo from a custom string.
    TypeInfo(const std::string& ti);

    std::string asString() const;
    std::string asDemangledString() const;

    //TODO: DIE
    const char* asCString() const;

    bool operator==(const TypeInfo& b) const;
    bool operator!=(const TypeInfo& b) const;
    bool operator<(const TypeInfo& b) const;

  private:
    const std::type_info* stdInfo;
    // C4251
    std::string           customInfo;
  };

  /** Interface for all the operations we need on any type:
   *
   *  - cloning/destruction in clone() and destroy()
   *  - Access to value from storage and storage creation in
   *    ptrFromStorage() and initializeStorage()
   *  - Type of specialized interface through kind()
   *
   * Our aim is to transport arbitrary values through:
   *  - synchronous calls: Nothing to do, values are just transported and
   *    converted.
   *  - asynchronous call/thread change: Values are copied.
   *  - process change: Values are serialized.
   *
   */
  class QITYPE_API TypeInterface
  {
  public:
    virtual const TypeInfo& info() =0;

    // Initialize and return a new storage, from nothing or a T*
    virtual void* initializeStorage(void* ptr=0)=0;

    // Get pointer to type from pointer to storage
    // Use a pointer and not a reference to avoid the case where the compiler makes a copy on the stack
    virtual void* ptrFromStorage(void**)=0;

    virtual void* clone(void*)=0;
    virtual void destroy(void*)=0;

    virtual TypeKind kind() const;

    // Less must always work: compare pointers if you have to.
    virtual bool less(void* a, void* b) = 0;

    //TODO: DIE
    inline const char* infoString() { return info().asCString(); } // for easy gdb access

    /** @return the serialization signature corresponding to what the type
     * would emit
     * @param resolveDynamic: if true, resolve dynamic types as deep as possible
     * for example a list<AnyReference> that happens to only contain int32
     * will return [i]
     * @warning if resolveDynamic is true, a valid storage must be given
    */
    qi::Signature signature(void* storage=0, bool resolveDynamic = false);

    ///@return a Type on which signature() returns sig.
    static TypeInterface* fromSignature(const qi::Signature& sig);
  };

  /// Runtime Type factory getter. Used by typeOf<T>()
  QITYPE_API TypeInterface*  getType(const std::type_info& type);

  /// Runtime Type factory setter.
  QITYPE_API bool registerType(const std::type_info& typeId, TypeInterface* type);

  /** Get type from a type. Will return a static TypeImpl<T> if T is not registered
   */
  template<typename T> TypeInterface* typeOf();

  /// Get type from a value. No need to delete the result
  template<typename T> TypeInterface* typeOf(const T& v)
  {
    return typeOf<T>();
  }


//MACROS

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

}


#endif  // _QITYPE_DETAILS_TYPEINTERFACE_HPP_

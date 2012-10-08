#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPE_HPP_
#define _QIMESSAGING_TYPE_HPP_

#include <typeinfo>
#include <string>

#include <boost/preprocessor.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/mpl/if.hpp>

#include <qi/log.hpp>
#include <qimessaging/api.hpp>

namespace qi{

class TypeInt;
class TypeFloat;
class TypeString;
class TypePointer;
class TypeTuple;
class ObjectType;
class GenericValue;
class GenericList;
class GenericMap;
class GenericObject;
class IDataStream;
class ODataStream;
class Signature;


/** This class is used to uniquely identify a type.
 *
 */
class QIMESSAGING_API TypeInfo
{
public:
  TypeInfo();
  /// Construct a TypeInfo from a std::type_info
  TypeInfo(const std::type_info& info);
  /// Contruct a TypeInfo from a custom string.
  TypeInfo(const std::string& ti);
  std::string asString();

  bool operator ==(const TypeInfo& b) const;
  bool operator !=(const TypeInfo& b) const;
  bool operator < (const TypeInfo& b) const;
private:
  const std::type_info* stdInfo;
  std::string customInfo;
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
      converted.
 *  - asynchronous call/thread change: Values are copied.
 *  - process change: Values are serialized.
 *
 */
class QIMESSAGING_API Type
{
public:
  virtual TypeInfo info() =0;
  // Initialize and return a new storage, from nothing or a T*
  virtual void* initializeStorage(void* ptr=0)=0;
  // Get pointer to type from pointer to storage
  // Use a pointer and not a reference to avoid the case where the compiler makes a copy on the stack
  virtual void* ptrFromStorage(void**)=0;

  virtual void* clone(void*)=0;
  virtual void destroy(void*)=0;

  enum Kind
  {
    Void,
    Int,
    Float,
    String,
    List,
    Map,
    Object,
    Pointer,
    Tuple,
    Dynamic,
    Unknown,
  };
  virtual Kind kind() const;

  std::string infoString() { return info().asString();} // for easy gdb access

  std::string signature();
  ///@return a Type on which signature() returns sig.
  static Type* fromSignature(const Signature& sig);

  GenericValue deserialize(IDataStream& in);
  void serialize(ODataStream& out, void* storage);

  /** Invoke one of the visitor functions in dispatcher based on kind().
   * Dispatcher must implement TypeDispatcher.
   * Passing 0 in storage is possible, in which case visitor functions will
   * get a dummy value but correct Type informations.
   */
  template<typename Dispatcher> Dispatcher& dispatch(const Dispatcher& disptacher, void** storage);
};

class QIMESSAGING_API TypeDispatcher
{
public:
  virtual void visitUnknown(Type* type, void* storage)=0;
  virtual void visitVoid(Type*)=0;
  virtual void visitInt(TypeInt* type, int64_t value, bool isSigned, int byteSize)=0;
  virtual void visitFloat(TypeFloat* type, double value, int byteSize)=0;
  virtual void visitString(TypeString* type, void* storage)=0;
  virtual void visitList(GenericList value)=0;
  virtual void visitMap(GenericMap value)=0;
  virtual void visitObject(GenericObject value)=0;
  virtual void visitPointer(TypePointer* type, void* storage, GenericValue pointee)=0;
  virtual void visitTuple(TypeTuple* type, void* storage)=0;
  virtual void visitDynamic(Type* type, GenericValue pointee);
};

/// Declare that a type has no accessible default constructor.
#define QI_TYPE_NOT_CONSTRUCTIBLE(T) \
namespace qi { namespace detail { \
template<> struct TypeManager<T>: public TypeManagerNonDefaultConstructible<T> {};}}

/// Declare that a type has no metatype and cannot be used in a Value
#define QI_NO_TYPE(T) namespace qi {template<> class TypeImpl<T> {};}

/// Declare that a type has no accessible copy constructor
#define QI_TYPE_NOT_CLONABLE(T) \
namespace qi { namespace detail { \
template<> struct TypeManager<T>: public TypeManagerNull<T> {};}}

/// Runtime Type factory getter. Used by typeOf<T>()
QIMESSAGING_API Type*  getType(const std::type_info& type);
/// Runtime Type factory setter.
QIMESSAGING_API bool registerType(const std::type_info& typeId, Type* type);

/// Register TypeImpl<t> in runtime type factory for 't'. Must be called from a .cpp file
#define QI_TYPE_REGISTER(t) \
  QI_TYPE_REGISTER_CUSTOM(t, qi::TypeImpl<t>)
/// Register 'typeimpl' in runtime type factory for 'type'.
#define QI_TYPE_REGISTER_CUSTOM(type, typeimpl) \
static bool BOOST_PP_CAT(__qi_registration, __LINE__) = qi::registerType(typeid(type), new typeimpl)

/** Get type from a type. Will return a static TypeImpl<T> if T is not registered
 */
template<typename T> Type* typeOf();

/// Get type from a value. No need to delete the result
template<typename T> Type* typeOf(const T& v)
{
  return typeOf<T>();
}

}
#include <qimessaging/details/typeimpl.hxx>
#include <qimessaging/details/type.hxx>

#endif  // _QIMESSAGING_TYPE_HPP_

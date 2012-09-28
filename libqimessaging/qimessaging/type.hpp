#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPE_HPP_
#define _QIMESSAGING_TYPE_HPP_

#include <typeinfo>
#include <boost/preprocessor.hpp>
#include <qimessaging/datastream.hpp>

namespace qi{

/** Interface for all the operations we need on any type:
 *
 *  - cloning/destruction in clone() and destroy()
 *  - Serialization through serialize() and deserialize() to transmit
 *    the value through some kind of pipe.
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
  virtual const std::type_info& info() =0;
  const char* infoString() { return info().name();} // for easy gdb access
  /// @return the serialization signature used by this type.
  virtual std::string signature()=0;

  // Initialize and return a new storage, from nothing or a T*
  virtual void* initializeStorage(void* ptr=0)=0;
  // Get pointer to type from pointer to storage
  // No reference to avoid the case where the compiler makes a copy on the stack
  virtual void* ptrFromStorage(void**)=0;

  virtual void* clone(void*)=0;
  virtual void destroy(void*)=0;

  // Default impl does toValue.serialize()
  virtual void  serialize(ODataStream& s, const void*)=0;
  // Default impl does deserialize(GenericValue&) then fromValue
  virtual void* deserialize(IDataStream& s)=0;

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
    Unknown,
  };
  virtual Kind kind() const;

  /* When someone makes a call with arguments that do not match the
   * target signature (ex: int vs float), we want to handle it.
   * For this, given the known correct signature S and known incorrect
   * GenericValue v, we want to be able to obtain a new Type T that
   * serializes with type S, and then try to convert o into type T.
   *
   * For this we need a map<Signature, Type> that we will feed with
   * known types.
   *
   */
  typedef std::map<std::string, Type*> TypeSignatureMap;
  static TypeSignatureMap& typeSignatureMap()
  {
    static TypeSignatureMap res;
    return res;
  }

  static Type* getCompatibleTypeWithSignature(const std::string& sig)
  {
    TypeSignatureMap::iterator i = typeSignatureMap().find(sig);
    if (i == typeSignatureMap().end())
      return 0;
    else
      return i->second;
  }

  static bool registerCompatibleType(const std::string& sig,
    Type* mt)
  {
    typeSignatureMap()[sig] = mt;
    return true;
  }

  #define QI_REGISTER_MAPPING(sig, type) \
  static bool BOOST_PP_CAT(_qireg_map_ , __LINE__) = ::qi::Type::registerCompatibleType(sig, \
    ::qi::typeOf<type>());
};


namespace detail {
  template<typename T> struct TypeFactory
  {
    void* operator()() { return new T();}
  };
};
#define QI_TYPE_NOT_CONSTRUCTIBLE(T) \
namespace qi { namespace detail { \
template<> struct TypeFactory<T> { void* operator()() { return 0;}};}}
/** Meta-type specialization.
 *  Use the aspect pattern, make a class per feature group
 *  (Clone, GenericValue, Serialize)
 *
 */

/// Access API that stores a T* in storage
template<typename T> class TypeDefaultAccess
{
public:
  typedef T type;
  static void* ptrFromStorage(void** storage)
  {
    return *storage;
  }
  static void* initializeStorage(void* ptr=0)
  {
    if (ptr)
      return ptr;
    void* res = detail::TypeFactory<T>()();
    if (!res)
      qiLogError("qi.meta") << "initializeStorage error on " << typeid(T).name();
    return res;
  }
};

/// Access api that stores T in storage
template<typename T> class TypeDirectAccess
{
public:
  typedef T type;
  static void* ptrFromStorage(void** storage)
  {
    return storage;
  }
  static void* initializeStorage(void* ptr=0)
  {
    if (ptr)
    {
      T val = *(T*)ptr;
      return *(void**)&val;
    }
    else
      return 0;
  }
};

template<typename A> class TypeDefaultClone
{
public:
  typedef typename A::type type;
  static void* clone(void* src)
  {
    const type* ptr = (const type*)A::ptrFromStorage(&src);
    void* res = A::initializeStorage();
    type* tres = (type*)A::ptrFromStorage(&res);
    *tres = *ptr;
    return res;
  }

  static void destroy(void* src)
  {
    type* ptr = (type*)A::ptrFromStorage(&src);
    delete ptr;
  }
};

template<typename A> class TypeNoClone
{
public:
  static void* clone(void* src)
  {
    return src;
  }

  static void destroy(void* ptr)
  {
    /* Assume a TypeNoClone is not serializable
     * So it cannot have been allocated by us.
     * So the destroy comes after a clone->ignore it
     */
  }
};

template<typename A> class TypeDefaultSerialize
{
public:
  typedef typename A::type type;
  static void  serialize(ODataStream& s, const void* src)
  {
    const type* ptr = (const type*)A::ptrFromStorage((void**)&src);
    s << *ptr;
  }

  static void* deserialize(IDataStream& s)
  {
    void* storage = A::initializeStorage();
    type* ptr = (type*)A::ptrFromStorage(&storage);
    s >> *ptr;
    return storage;
  }
  static std::string signature()
  {
    return signatureFromType<type>::value();
  }
};

template<typename T> class TypeNoSerialize
{
public:
  static void serialize(ODataStream& s, const void* ptr)
  {
    qiLogWarning("qi.meta") << "serialize not implemented for " << typeid(T).name();
  }

  static void* deserialize(IDataStream& s)
  {
    qiLogWarning("qi.meta") << "deserialize not implemented for " << typeid(T).name();
    return 0;
  }
 static  std::string signature()
  {
    std::string res;
    res += (char)Signature::Type_Unknown;
    return res;
  }
};


/* implementation of Type methods that bounces to the various aspect
 * subclasses.
 *
 * It does not inherit from Type on purpose, to avoid diamond inheritance
 * when specializing the type interface
 * That way we can split the various aspects in different classes
 * for better reuse, without the cost of a second virtual call.
 */
template<typename T, typename _Access    = TypeDefaultAccess<T>
                   , typename _Cloner    = TypeDefaultClone<_Access>
                   , typename _Serialize = TypeNoSerialize<_Access>
         > class DefaultTypeImplMethods
{
public:
  typedef _Access Access;
  typedef _Cloner Cloner;
  typedef _Serialize Serialize;
  static void* initializeStorage(void* ptr=0)
  {
    return Access::initializeStorage(ptr);
  }

  static void* ptrFromStorage(void** storage)
  {
    return Access::ptrFromStorage(storage);
  }

  static const std::type_info& info()
  {
    return typeid(T);
  }

  static void* clone(void* src)
  {
    return Cloner::clone(src);
  }

  static void destroy(void* ptr)
  {
    Cloner::destroy(ptr);
  }

  static std::string signature()
  {
    return Serialize::signature();
  }

  static void  serialize(ODataStream& s, const void* ptr)
  {
    Serialize::serialize(s, ptr);
  }

  static void* deserialize(IDataStream& s)
  {
    return Serialize::deserialize(s);
  }
};

#define _QI_BOUNCE_TYPE_METHODS(Bounce)                                                             \
virtual const std::type_info& info() { return Bounce::info();}                                      \
virtual std::string signature() { return Bounce::signature();}                                      \
virtual void* initializeStorage(void* ptr=0) { return Bounce::initializeStorage(ptr);}              \
virtual void* ptrFromStorage(void**s) { return Bounce::ptrFromStorage(s);}                          \
virtual void* clone(void* ptr) { return Bounce::clone(ptr);}                                        \
virtual void destroy(void* ptr) { return Bounce::destroy(ptr);}                                     \
virtual void  serialize(ODataStream& s, const void* p) { return Bounce::serialize(s, p);}           \
virtual void* deserialize(IDataStream& s) { return Bounce::deserialize(s);}

template<typename T, typename _Access    = TypeDefaultAccess<T>
                   , typename _Cloner    = TypeDefaultClone<_Access>
                   , typename _Serialize = TypeNoSerialize<_Access>
                   > class DefaultTypeImpl
                   : public Type
                   {
                   public:
                     typedef DefaultTypeImplMethods<T, _Access, _Cloner, _Serialize> MethodsImpl;
                     _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
                   };

/* Type "factory". Specialize this class to provide a custom
 * Type for a given type.
 */
template<typename T> class TypeImpl: public virtual DefaultTypeImpl<T>
{
};

/** Declare that a type is serialisable
 *  Must be called outside any namespace.
 */
#define QI_TYPE_SERIALIZABLE(T)              \
namespace qi {                                   \
template<> class TypeImpl<T>:                \
  public DefaultTypeImpl<T,                  \
    TypeDefaultAccess<T>,                    \
    TypeDefaultClone<TypeDefaultAccess<T> >, \
    TypeDefaultSerialize<TypeDefaultAccess<T> >   \
  >{}; }

/// Declare that a type has no metatype and cannot be used in a Value
#define QI_NO_TYPE(T) namespace qi {template<> class TypeImpl<T> {};}

/// Declare that a type with default TypeImpl is not clonable
#define QI_TYPE_NOT_CLONABLE(T) \
namespace qi {    \
  template<> struct TypeDefaultClone<TypeDefaultAccess<T> >: public TypeNoClone<TypeDefaultAccess<T> >{}; \
}

/// Type factory getter. All other type access mechanism bounce here
QIMESSAGING_API Type*  getType(const std::type_info& type);
/// Type factory setter
QIMESSAGING_API bool registerType(const std::type_info& typeId, Type* type);

/** Get type from a type. Will return a static TypeImpl<T> if T is not registered
 */
template<typename T> Type* typeOf();

/// Get type from a value. No need to delete the result
template<typename T> Type* typeOf(const T& v)
{
  return typeOf<T>();
}

}

QI_TYPE_SERIALIZABLE(Buffer);

#include <qimessaging/details/type.hxx>

#endif  // _QIMESSAGING_TYPE_HPP_

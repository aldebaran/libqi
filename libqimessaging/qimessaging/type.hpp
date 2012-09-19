#ifndef _QIMESSAGING_TYPE_HPP_
#define _QIMESSAGING_TYPE_HPP_

#include <typeinfo>
#include <boost/preprocessor.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/details/dynamicvalue.hpp>

namespace qi{

/** Interface for all the operations we need on any type:
 *
 *  - cloning/destruction in clone() and destroy()
 *  - type conversion is made by going through the generic container
 *    GenericValue, using the toValue() and fromValue() functions,
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

  virtual void* clone(void*)=0;
  virtual void destroy(void*)=0;

  virtual bool  toValue(const void*, qi::detail::DynamicValue&)=0;
  virtual void* fromValue(const qi::detail::DynamicValue&)=0;

  // Default impl does toValue.serialize()
  virtual void  serialize(ODataStream& s, const void*)=0;
  // Default impl does deserialize(GenericValue&) then fromValue
  virtual void* deserialize(IDataStream& s)=0;

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

/** Meta-type specialization.
 *  Use the aspect pattern, make a class per feature group
 *  (Clone, GenericValue, Serialize)
 *
 */

template<typename T> class TypeDefaultClone
{
public:
  void* clone(void* src)
  {
    return new T(*(T*)src);
  }

  void destroy(void* ptr)
  {
    delete (T*)ptr;
  }
};

template<typename T> class TypeNoClone
{
public:
  void* clone(void* src)
  {
    return src;
  }

  void destroy(void* ptr)
  {
    /* Assume a TypeNoClone is not serializable
     * So it cannot have been allocated by us.
     * So the destroy comes after a clone->ignore it
     */
  }
};

template<typename T>class TypeDefaultValue
{
public:
  bool toValue(const void* ptr, qi::detail::DynamicValue& val)
  {
    detail::DynamicValueConverter<T>::writeDynamicValue(*(T*)ptr, val);
    return true;
  }

  void* fromValue(const qi::detail::DynamicValue& val)
  {
    T* res = new T();
    detail::DynamicValueConverter<T>::readDynamicValue(val, *res);
    return res;
  }
};

template<typename T>class TypeNoValue
{
public:
  bool toValue(const void* ptr, qi::detail::DynamicValue& val)
  {
    qiLogWarning("qi.type") << "toValue not implemented for type ";
    return false;
  }

  void* fromValue(const qi::detail::DynamicValue& val)
  {
    qiLogWarning("qi.type") << "fromValue not implemented for type ";
    T* res = new T();
    return res;
  }
};

template<typename T> class TypeDefaultSerialize
{
public:
  void  serialize(ODataStream& s, const void* ptr)
  {
    s << *(T*)ptr;
  }

  void* deserialize(IDataStream& s)
  {
    T* val = new T();
    s >> *val;
    return val;
  }
  std::string signature()
  {
    return signatureFromType<T>::value();
  }
};

template<typename T> class TypeNoSerialize
{
public:
  void serialize(ODataStream& s, const void* ptr)
  {
    qiLogWarning("qi.meta") << "type not serializable";
  }

  void* deserialize(IDataStream& s)
  {
    qiLogWarning("qi.meta") << "type not serializable";
    T* val = new T();
    return val;
  }
  std::string signature()
  {
    std::string res;
    res += (char)Signature::Type_Unknown;
    return res;
  }
};


/* TypeImpl implementation that bounces to the various aspect
 * subclasses.
 *
 * That way we can split the various aspects in different classes
 * for better reuse, without the cost of a second virtual call.
 */
template<typename T, typename Cloner    = TypeDefaultClone<T>
                   , typename Value     = TypeNoValue<T>
                   , typename Serialize = TypeNoSerialize<T>
         > class DefaultTypeImpl
: public Cloner
, public Value
, public Serialize
, public virtual Type
{
  virtual const std::type_info& info()
  {
    static T v; return typeid(v);
  }

  virtual void* clone(void* src)
  {
    return Cloner::clone(src);
  }

  virtual void destroy(void* ptr)
  {
    Cloner::destroy(ptr);
  }

  virtual bool toValue(const void* ptr, qi::detail::DynamicValue& val)
  {
    return Value::toValue(ptr, val);
  }

  virtual void* fromValue(const qi::detail::DynamicValue& val)
  {
    return Value::fromValue(val);
  }

  virtual std::string signature()
  {
    return Serialize::signature();
  }

  virtual void  serialize(ODataStream& s, const void* ptr)
  {
    Serialize::serialize(s, ptr);
  }

  virtual void* deserialize(IDataStream& s)
  {
    return Serialize::deserialize(s);
  }
};

/* Type "factory". Specialize this class to provide a custom
 * Type for a given type.
 */
template<typename T> class TypeImpl: public virtual DefaultTypeImpl<T>
{
};

/// Declare a type that is convertible to GenericValue, and serializable
#define QI_TYPE_CONVERTIBLE_SERIALIZABLE(T)  \
namespace qi {                                   \
template<> class TypeImpl<T>:                \
  public DefaultTypeImpl<T,                  \
    TypeDefaultClone<T>,                     \
    TypeDefaultValue<T>,                     \
    TypeDefaultSerialize<T>                  \
  >{}; }

/** Declare that a type is not convertible to GenericValue.
 *  Must be called outside any namespace.
 */
#define QI_TYPE_SERIALIZABLE(T)              \
namespace qi {                                   \
template<> class TypeImpl<T>:                \
  public DefaultTypeImpl<T,                  \
    TypeDefaultClone<T>,                     \
    TypeNoValue<T>,                          \
    TypeDefaultSerialize<T>                  \
  >{}; }

/// Declare that a type has no metatype and cannot be used in a Value
#define QI_NO_TYPE(T) namespace qi {template<> class TypeImpl<T> {};}



/// Get type from a type. No need to delete the result
template<typename T> Type* typeOf()
{
  static TypeImpl<typename boost::remove_const<T>::type> res;
  return &res;
}

/// Get type from a value. No need to delete the result
template<typename T> Type* typeOf(const T& v)
{
  return typeOf<T>();
}

}

QI_TYPE_SERIALIZABLE(Buffer);

#include <qimessaging/details/type.hxx>

#endif

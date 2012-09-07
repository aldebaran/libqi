#ifndef _QIMESSAGING_METATYPE_HPP_
#define _QIMESSAGING_METATYPE_HPP_

#include <typeinfo>
#include <boost/preprocessor.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/details/value.hpp>

namespace qi{

/** Interface for all the operations we need on any type:
 *
 *  - cloning/destruction in clone() and destroy()
 *  - type conversion is made by going through the generic container
 *    Value, using the toValue() and fromValue() functions,
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
class QIMESSAGING_API MetaType
{
public:
  virtual const std::type_info& info() =0;
  /// @return the serialization signature used by this type.
  virtual std::string signature()=0;

  virtual void* clone(void*)=0;
  virtual void destroy(void*)=0;

  virtual bool  toValue(const void*, qi::detail::Value&)=0;
  virtual void* fromValue(const qi::detail::Value&)=0;

  // Default impl does toValue.serialize()
  virtual void  serialize(ODataStream& s, const void*)=0;
  // Default impl does deserialize(Value&) then fromValue
  virtual void* deserialize(IDataStream& s)=0;

  /* When someone makes a call with arguments that do not match the
   * target signature (ex: int vs float), we want to handle it.
   * For this, given the known correct signature S and known incorrect
   * MetaValue v, we want to be able to obtain a new MetaType T that
   * serializes with type S, and then try to convert o into type T.
   *
   * For this we need a map<Signature, MetaType> that we will feed with
   * known types.
   *
   */
  typedef std::map<std::string, MetaType*> MetaTypeSignatureMap;
  static MetaTypeSignatureMap& metaTypeSignatureMap()
  {
    static MetaTypeSignatureMap res;
    return res;
  }

  static MetaType* getCompatibleTypeWithSignature(const std::string& sig)
  {
    MetaTypeSignatureMap::iterator i = metaTypeSignatureMap().find(sig);
    if (i == metaTypeSignatureMap().end())
      return 0;
    else
      return i->second;
  }

  static bool registerCompatibleType(const std::string& sig,
    MetaType* mt)
  {
    metaTypeSignatureMap()[sig] = mt;
    return true;
  }

  #define QI_REGISTER_MAPPING(sig, type) \
  static bool BOOST_PP_CAT(_qireg_map_ , __LINE__) = ::qi::MetaType::registerCompatibleType(sig, \
    ::qi::metaTypeOf<type>());
};


/** Meta-type specialization.
 *  Use the aspect pattern, make a class per feature group
 *  (Clone, Value, Serialize)
 *
 */

template<typename T> class MetaTypeDefaultClone
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

template<typename T>class MetaTypeDefaultValue
{
public:
  bool toValue(const void* ptr, qi::detail::Value& val)
  {
    detail::ValueConverter<T>::writeValue(*(T*)ptr, val);
    return true;
  }

  void* fromValue(const qi::detail::Value& val)
  {
    T* res = new T();
    detail::ValueConverter<T>::readValue(val, *res);
    return res;
  }
};

template<typename T>class MetaTypeNoValue
{
public:
  bool toValue(const void* ptr, qi::detail::Value& val)
  {
    return false;
  }

  void* fromValue(const qi::detail::Value& val)
  {
    T* res = new T();
    return res;
  }
};

template<typename T> class MetaTypeDefaultSerialize
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
};

/* MetaTypeImpl implementation that bounces to the various aspect
 * subclasses.
 *
 * That way we can split the various aspects in different classes
 * for better reuse, without the cost of a second virtual call.
 */
template<typename T, typename Cloner    = MetaTypeDefaultClone<T>
                   , typename Value     = MetaTypeDefaultValue<T>
                   , typename Serialize = MetaTypeDefaultSerialize<T>
         > class DefaultMetaTypeImpl
: public Cloner
, public Value
, public Serialize
, public MetaType
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

  virtual bool toValue(const void* ptr, qi::detail::Value& val)
  {
    return Value::toValue(ptr, val);
  }

  virtual void* fromValue(const qi::detail::Value& val)
  {
    return Value::fromValue(val);
  }

  virtual std::string signature()
  {
    return signatureFromType<T>::value();
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

/* MetaType "factory". Specialize this class to provide a custom
 * MetaType for a given type.
 */
template<typename T> class MetaTypeImpl: public DefaultMetaTypeImpl<T>
{
};

/** Declare that a type is not convertible to Value.
 *  Must be called outside any namespace.
 */
#define QI_METATYPE_NOT_CONVERTIBLE(T)           \
namespace qi {                                   \
template<> class MetaTypeImpl<T>:                \
  public DefaultMetaTypeImpl<T,                  \
    MetaTypeDefaultClone<T>,                     \
    MetaTypeNoValue<T>,                          \
    MetaTypeDefaultSerialize<T>                  \
  >{}; }

/// Declare that a type has no metatype and cannot be converted to MetaValue
#define QI_NO_METATYPE(T) namespace qi {template<> class MetaTypeImpl<T> {};}

/// Get metaType from a value. No need to delete the result
template<typename T> MetaType* metaTypeOf(const T& v)
{
  static MetaTypeImpl<T> res;
  return &res;
}

/// Get metaType from a type. No need to delete the result
template<typename T> MetaType* metaTypeOf()
{
  T* t = 0;
  return metaTypeOf(*t);
}

}

QI_METATYPE_NOT_CONVERTIBLE(Buffer);

#include <qimessaging/details/metatype.hxx>

#endif

#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_DETAIL_OBJECT_HXX_
#define _QI_TYPE_DETAIL_OBJECT_HXX_

#include <functional>

#include <qi/future.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/type/typeobject.hpp>
#include <qi/type/detail/typeimpl.hxx>
#include <qi/type/detail/genericobject.hpp>
#include <qi/type/metasignal.hpp>
#include <qi/type/metamethod.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/objectuid.hpp>

// Visual defines interface...
#ifdef interface
#undef interface
#endif

#include <type_traits>

namespace qi {

// This id is used to identify a null Object. It is useful for example in Objects serialization.
static const unsigned int nullObjectId = 0;

class Empty {};

namespace detail {
  using ProxyGeneratorMap = std::map<TypeInfo, boost::function<AnyReference(AnyObject)>>;
  QI_API ProxyGeneratorMap& proxyGeneratorMap();

  /* On ubuntu (and maybe other platforms), the linking is done by default with
   * --as-needed.
   * Proxy libraries constist only of static initialization functions which add
   * the proxy factories to the global maps, thus there is no direct dependency
   * between the program and the library (no function call whatsoever) and the
   * dependency gets dropped by the linker.
   * This class is specialized when including interfaces which have proxies
   * and the function is defined in the .so to force a dependency.
   */
  template <typename T>
  struct ForceProxyInclusion
  {
    bool dummyCall() { return true; }
  };

  // bounce to a genericobject obtained by (O*)this->asAnyObject()
  /* Everything need to be const:
   * anyobj.call bounces to anyobj.asObject().call, and the
   * second would work on const object
   */
  template<typename O> class GenericObjectBounce
  {
  public:
    const MetaObject &metaObject() const { return go()->metaObject(); }
    inline qi::Future<AnyReference> metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto, Signature returnSignature=Signature()) const
    {
      return go()->metaCall(method, params, callType, returnSignature);
    }
    inline int findMethod(const std::string& name, const GenericFunctionParameters& parameters) const
    {
      return go()->findMethod(name, parameters);
    }
    inline qi::Future<AnyReference> metaCall(const std::string &nameWithOptionalSignature, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto, Signature returnSignature=Signature()) const
    {
      return go()->metaCall(nameWithOptionalSignature, params, callType, returnSignature);
    }
    inline void metaPost(unsigned int event, const GenericFunctionParameters& params) const
    {
      return go()->metaPost(event, params);
    }
    inline void metaPost(const std::string &nameWithOptionalSignature, const GenericFunctionParameters &in) const
    {
      return go()->metaPost(nameWithOptionalSignature, in);
    }
    template <typename... Args>
    inline void post(const std::string& eventName, Args&&... args) const
    {
      return go()->post(eventName, std::forward<Args>(args)...);
    }
    template <typename FUNCTOR_TYPE>
    inline qi::FutureSync<SignalLink> connect(const std::string& eventName, FUNCTOR_TYPE callback,
      MetaCallType threadingModel = MetaCallType_Auto) const
    {
      return go()->connect(eventName, callback, threadingModel);
    }
    inline qi::FutureSync<SignalLink> connect(const std::string &name, const SignalSubscriber& functor) const
    {
      return go()->connect(name, functor);
    }
    inline qi::FutureSync<SignalLink> connect(unsigned int signal, const SignalSubscriber& subscriber) const
    {
      return go()->connect(signal, subscriber);
    }
    // Cannot inline here, AnyObject not fully declared
    qi::FutureSync<SignalLink> connect(unsigned int signal, AnyObject target, unsigned int slot) const;
    inline qi::FutureSync<void> disconnect(SignalLink linkId) const
    {
      return go()->disconnect(linkId);
    }
    template<typename T>
    inline qi::FutureSync<T> property(const std::string& name) const
    {
      return go()->template property<T>(name);
    }
    template<typename T>
    inline qi::FutureSync<void> setProperty(const std::string& name, const T& val) const
    {
      return go()->setProperty(name, val);
    }
    inline qi::FutureSync<AnyValue> property(unsigned int id) const
    {
      return go()->property(id);
    }
    inline qi::FutureSync<void> setProperty(unsigned int id, const AnyValue &val) const
    {
      return go()->setProperty(id, val);
    }
    inline ExecutionContext* executionContext() const
    {
      return go()->executionContext().get();
    }
    inline bool isStatsEnabled() const
    {
      return go()->isStatsEnabled();
    }
    inline void enableStats(bool enable) const
    {
      return go()->enableStats(enable);
    }
    inline ObjectStatistics stats() const
    {
      return go()->stats();
    }
    inline void clearStats() const
    {
      return go()->clearStats();
    }
    inline bool isTraceEnabled() const
    {
      return go()->isTraceEnabled();
    }
    inline void enableTrace(bool enable)
    {
      return go()->enableTrace(enable);
    }
    inline void forceExecutionContext(boost::shared_ptr<qi::ExecutionContext> ec)
    {
      return go()->forceExecutionContext(ec);
    }
    template <typename R, typename... Args>
    qi::Future<R> async(const std::string& methodName, Args&&... args) const
    {
      return go()->template async<R>(methodName, std::forward<Args>(args)...);
    }
    template <typename R, typename... Args>
    R call(const std::string& methodName, Args&&... args) const
    {
      return go()->template call<R>(methodName, std::forward<Args>(args)...);
    }

  private:
    inline GenericObject* go() const
    {
      GenericObject* g = static_cast<const O*>(this)->asGenericObject();
      if (!g)
        throw std::runtime_error("This object is null");
      return g;
    }
  };

  template <typename T>
  struct InterfaceImplTraits
  {
    using Defined = std::false_type;
  };
}

// these methods are used by advertiseFactory and arguments are specified explicitely, we can't used forwarding here
template <typename T, typename... Args>
typename boost::enable_if<typename detail::InterfaceImplTraits<T>::Defined, qi::Object<T> >::type constructObject(
    Args... args)
{
  return boost::make_shared<typename detail::InterfaceImplTraits<T>::ImplType>(std::forward<Args>(args)...);
}
template <typename T, typename... Args>
typename boost::disable_if<typename detail::InterfaceImplTraits<T>::Defined, qi::Object<T> >::type constructObject(
    Args&&... args)
{
  return Object<T>(new T(std::forward<Args>(args)...));
}

// QI_REGISTER_IMPLEMENTATION_H associates an interface type to an
// implementation type that might be unrelated (no inheritance or conversion).
// However, not like QI_REGISTER_IMPLEMENTATION(note the absence of `_H`),
// QI_REGISTER_IMPLEMENTATION_H also generates special proxy types wrapping
// the real implementation object.
// These wrappers are then used as implementation details by libqi
// when an object of the implementation type is stored in a qi::Object.
// The wrappers handle the thread-safety and the different kinds of API
// (returning or not futures, for example) while not modifying the original type.
#define QI_REGISTER_IMPLEMENTATION_H(interface, impl)\
namespace qi\
{\
  namespace detail\
  {\
    template <>\
    struct InterfaceImplTraits<interface>\
    {\
      using Defined = std::true_type;\
      using InterfaceType = interface;\
      using ImplType = impl;\
      using AsyncType = interface##LocalAsync<boost::shared_ptr<ImplType>>;\
      using SyncType = interface##LocalSync<boost::shared_ptr<ImplType>>;\
    };\
\
    template<>\
    struct InterfaceImplTraits<impl>:\
      public InterfaceImplTraits<interface>\
    {};\
  }\
}

/** Type erased object that has a known interface T.
 *
 * In case T is unknown, you can use qi::AnyObject which aliases to
 * Object<qi::Empty>.
 *
 * You can then use the object with type-erasure or call the object directly
 * using the operator ->.
 *
 * \includename{qi/anyobject.hpp}
 */
template<typename T> class Object :
  public detail::GenericObjectBounce<Object<T>>
{
  // see qi::Future constructors below
  struct None {
    detail::ManagedObjectPtr _obj;
  };
public:
  Object();

  template<typename U> Object(const Object<U>& o);
  template<typename U> Object<T>& operator=(const Object<U>& o);
  // Templates above do not replace default ctor or copy operator
  Object(const Object& o);
  Object<T>& operator=(const Object& o);
  // Disable the ctor taking future if T is Empty, as it would conflict with
  // We use None to disable it. The method must be instantiable because when we
  // export the class under windows, all functions are instanciated
  // Future cast operator
  using MaybeAnyObject = std::conditional_t<std::is_same_v<T, Empty>, None, Object<Empty>>;
  Object(const qi::Future<MaybeAnyObject>& fobj);
  Object(const qi::FutureSync<MaybeAnyObject>& fobj);

  /// @{

  /** These constructors take ownership of the underlying pointers.
  * If a callback is given, it will be called instead of the default
  * behavior of deleting the stored GenericObject and the underlying T object.
  */

  Object(GenericObject* go);
  Object(T* ptr);
  Object(GenericObject* go, boost::function<void(GenericObject*)> deleter);
  Object(T* ptr, boost::function<void(T*)> deleter);
  /// @}

  explicit Object(detail::ManagedObjectPtr obj)
  {
    init(obj);
  }

  /// Shares ref counter with other, which must handle the destruction of go.
  ///
  /// \deprecated This constructor is considered harmful. It is undefined
  /// behavior to associate a `GenericObject` to a different `shared_ptr` than
  /// the one it was instantiated with.
  //
  // WARNING: This constructor relies on the use of an internal function of the
  // `boost::shared_ptr` API.
  // TODO: Remove this constructor.
  template <typename U>
  QI_API_DEPRECATED_MSG("This constructor is considered harmful.")
  Object(GenericObject* go, boost::shared_ptr<U> other);

  template<typename U> Object(boost::shared_ptr<U> other);
  bool isValid() const;
  explicit operator bool() const;
  operator Object<Empty>() const;

  // Generates `>`, `<=`, `>=` in terms of `<`.
  friend KA_GENERATE_REGULAR_OP_GREATER(Object)
  friend KA_GENERATE_REGULAR_OP_LESS_OR_EQUAL(Object)
  friend KA_GENERATE_REGULAR_OP_GREATER_OR_EQUAL(Object)

  /// The unique identifier of the object qi::Object's instance is refering to.
  /// Won't change if that instance travels through the network.
  ObjectUid uid() const;

  QI_API_DEPRECATED_MSG("Use qi::Object::uid() instead.")
  PtrUid ptrUid() const { return uid(); }

  boost::shared_ptr<T> asSharedPtr();

  T& asT() const;
  T* operator->() const;
  T& operator *() const;
  bool unique() const;
  GenericObject* asGenericObject() const;
  void reset();
  unsigned use_count() const { return _obj.use_count();}

  static ObjectTypeInterface* interface();
  // Check or obtain T interface, or throw
  void checkT();
  // no-op deletor callback
  static void keepManagedObjectPtr(detail::ManagedObjectPtr) {}
  static void noDeleteT(T*) {qiLogDebug("qi.object") << "AnyObject noop T deleter";}
  static void noDelete(GenericObject*) {qiLogDebug("qi.object") << "AnyObject noop deleter";}
  // deletor callback that deletes only the GenericObject and not the content
  static void deleteGenericObjectOnly(GenericObject* obj) { qiLogDebug("qi.object") << "AnyObject GO deleter"; delete obj;}

  static void deleteCustomDeleter(GenericObject* obj, boost::function<void(T*)> deleter)
  {
    qiLogDebug("qi.object") << "custom deleter";
    deleter((T*)obj->value);
    delete obj;
  }
  detail::ManagedObjectPtr managedObjectPtr() { return _obj;}
private:
  friend class GenericObject;

  template <typename> friend class Object;
  template <typename> friend class WeakObject;

  void init(detail::ManagedObjectPtr obj);

  static void deleteObject(GenericObject* obj)
  {
    qiLogDebug("qi.object") << "deleteObject " << obj << " "
      << obj->value << " " << obj->metaObject().description();
    obj->type->destroy(obj->value);
    delete obj;
  }

  /* Do not change this, Object<T> must be binary-equivalent to
   * ManagedObjectPtr.
   */
  detail::ManagedObjectPtr _obj;
};

template<typename T> class WeakObject
{
public:
  WeakObject() {}
  template<typename U> WeakObject(const Object<U>& o)
  : _ptr(o._obj) {}
  Object<T> lock() { return Object<T>(_ptr.lock());}
  boost::weak_ptr<GenericObject> _ptr;
};
using AnyWeakObject = WeakObject<Empty>;

template<typename T> inline ObjectTypeInterface* Object<T>::interface()
{
  TypeInterface* type = typeOf<T>();
  if (type->kind() != TypeKind_Object)
  {
    std::stringstream err;
    err << "Object<T> can only be used on registered object types. ("
    << type->infoString() << ")(" << type->kind() << ')';
    throw std::runtime_error(err.str());
  }
  ObjectTypeInterface* otype = static_cast<ObjectTypeInterface*>(type);
  return otype;
}

template<typename T> inline Object<T>::Object() {}

template<typename T>
template<typename U>
inline Object<T>::Object(const Object<U>& o)
{
  static bool unused = qi::detail::ForceProxyInclusion<T>().dummyCall();
  (void)unused;

  /* An Object<T> created by convert may be in fact an object that does
  * not implement the T interface.
  * Checking and converting on first access to T& is not enough:
  *  Object<Iface> o = obj.call("fetchOne"); // this one might be incorrect
  *  someVector.push_back(o); // pushes the incorrect one
  *  o->someIfaceOperation(); // will upgrade o, but not the one in someVector
  *
  * So we check as early as we can, in all copy pathes, and back-propagate
  * the upgrade to the source of the copy
  */
  const_cast<Object<U>&>(o).checkT();
  init(o._obj);
}
template<typename T>
template<typename U>
inline Object<T>& Object<T>::operator=(const Object<U>& o)
{
  static bool unused = qi::detail::ForceProxyInclusion<T>().dummyCall();
  (void)unused;

  const_cast<Object<U>&>(o).checkT();
  init(o._obj);

  return *this;
}
template<typename T> inline Object<T>::Object(const Object<T>& o)
{
  const_cast<Object<T>&>(o).checkT();
  init(o._obj);
}
template<typename T> inline Object<T>& Object<T>::operator=(const Object<T>& o)
{
  if (this == &o)
    return *this;

  const_cast<Object<T>&>(o).checkT();
  init(o._obj);

  return *this;
}
template<typename T> inline Object<T>::Object(GenericObject* go)
{
  init(detail::ManagedObjectPtr(go, &deleteObject));
}
template<typename T> inline Object<T>::Object(GenericObject* go, boost::function<void(GenericObject*)> deleter)
{
  init(detail::ManagedObjectPtr(go, deleter));
}

template<typename T> template<typename U> Object<T>::Object(GenericObject* go, boost::shared_ptr<U> other)
{
  init(detail::ManagedObjectPtr(other, go));

  // WARNING: Relying on an internal function from boost is codesmell and
  // dangerous. `GenericObject` inherits from `enable_shared_from_this` which
  // means constructing a `shared_ptr<GenericObject>` by aliasing and then
  // using `shared_from_this` is undefined behavior. This is why the following
  // call had to be added. This constructor is impossible to implement correctly
  // without this call and is therefore deprecated.
  // TODO: Remove this this constructor as soon as possible.
  //
  // Notify the shared_from_this of GenericObject
  _obj->_internal_accept_owner(&other, go);
}

namespace detail
{
  // Low-level constructor from type, for factorization purpose only
  template<typename T>
  ManagedObjectPtr managedObjectFromSharedPtr(
      ObjectTypeInterface* oit,
      boost::shared_ptr<T> other,
      const boost::optional<ObjectUid>& maybeUid = boost::none)
  {
    return ManagedObjectPtr(
            new GenericObject(oit, other.get(), maybeUid),
            [other](GenericObject* object) mutable {
              other.reset();
              delete object;
            });
  }

  // Constructing an AnyObject from a registered implementation.
  template<typename From>
  ManagedObjectPtr managedObjectFromSharedPtr(
      Object<Empty>&,
      boost::shared_ptr<From>& other,
      std::true_type)
  { // Bounce on a specialized object constructor
    return qi::Object<typename qi::detail::InterfaceImplTraits<From>::InterfaceType>(other).managedObjectPtr();
  }

  // Constructing an AnyObject from an arbitrary type
  template<typename From>
  ManagedObjectPtr managedObjectFromSharedPtr(
      Object<Empty>&,
      boost::shared_ptr<From>& other,
      std::false_type)
  { // Bounce on a specialized object constructor
    return qi::Object<From>(other).managedObjectPtr();
  }

  // Directly constructing object of the same type
  template<typename ToSameAsFrom>
  ManagedObjectPtr managedObjectFromSharedPtr(
      Object<ToSameAsFrom>& dst,
      boost::shared_ptr<ToSameAsFrom>& other,
      std::false_type)
  { // It may throw if type is not registered
    return managedObjectFromSharedPtr(dst.interface(), other);
  }

  // Original pointer is not recognized as an implementation to construct a specialized object
  template<typename To, typename From>
  ManagedObjectPtr managedObjectFromSharedPtr(
      Object<To>& dst,
      boost::shared_ptr<From>& other,
      std::false_type)
  {
    static_assert(
          std::is_base_of<typename std::decay<To>::type, typename std::decay<From>::type>::value,
          "Cannot construct object from instance, because it is not a direct base or an associated implementation");
    auto asDestinationType = boost::static_pointer_cast<To>(other);
    return managedObjectFromSharedPtr(dst.interface(), asDestinationType);
  }

  // Original pointer is recognized as an implementation to construct a specialized object
  template<typename To, typename From>
  ManagedObjectPtr managedObjectFromSharedPtr(
      Object<To>&,
      boost::shared_ptr<From>& other,
      std::true_type)
  {
    using SyncProxyType = typename InterfaceImplTraits<From>::SyncType;
    // We want the uid value to match the real impl object, not the proxy object we are using here.
    const auto realImplUid = os::ptrUid(other.get());
    auto localProxy = boost::make_shared<SyncProxyType>(other);
    return managedObjectFromSharedPtr(qi::Object<To>::interface(), std::move(localProxy), realImplUid);
  }
}

template<typename T> template<typename U> Object<T>::Object(boost::shared_ptr<U> other)
{
  _obj = detail::managedObjectFromSharedPtr(
        *this, other, typename qi::detail::InterfaceImplTraits<U>::Defined{});
}

template<typename T> inline Object<T>::Object(T* ptr)
{
  ObjectTypeInterface* otype = interface();
  _obj = detail::ManagedObjectPtr(new GenericObject(otype, ptr), &deleteObject);
}
template<typename T> inline Object<T>::Object(T* ptr, boost::function<void(T*)> deleter)
{
  ObjectTypeInterface* otype = interface();
  if (deleter)
    _obj = detail::ManagedObjectPtr(new GenericObject(otype, ptr),
      boost::bind(&Object::deleteCustomDeleter, _1, deleter));
  else
    _obj = detail::ManagedObjectPtr(new GenericObject(otype, ptr), &deleteObject);
}
template<typename T> inline Object<T>::Object(const qi::Future<MaybeAnyObject>& fobj)
{
  static bool unused = qi::detail::ForceProxyInclusion<T>().dummyCall();
  (void)unused;

  init(fobj.value()._obj);
}
template<typename T> inline Object<T>::Object(const qi::FutureSync<MaybeAnyObject>& fobj)
{
  static bool unused = qi::detail::ForceProxyInclusion<T>().dummyCall();
  (void)unused;

  init(fobj.value()._obj);
}

template<typename T> inline boost::shared_ptr<T> Object<T>::asSharedPtr()
{
  checkT();
  return boost::shared_ptr<T>(&asT(), boost::bind(&keepManagedObjectPtr, _obj));
}

template<typename T> inline void Object<T>::init(detail::ManagedObjectPtr obj)
{
  _obj = obj;
  if (!boost::is_same<T, Empty>::value && obj)
    checkT();
  _obj = obj;
}

/// Compares identities, not values.
///
/// The uid identifies the pointer initially used to create the Object. Thus,
/// if an object crosses the network boundaries, its (local) memory address will
/// change but the uid will be the same.
template<typename T, typename U>
bool operator==(const Object<T>& a, const Object<U>& b)
{
  QI_ASSERT(a.asGenericObject() && b.asGenericObject());
  return a.asGenericObject()->uid == b.asGenericObject()->uid;
}

template<typename T, typename U>
bool operator!=(const Object<T>& a, const Object<U>& b)
{
  return !(a == b);
}

template<typename T>
bool operator<(const Object<T>& a, const Object<T>& b)
{
  QI_ASSERT(a.asGenericObject() && b.asGenericObject());
  return a.asGenericObject()->uid < b.asGenericObject()->uid;
}

template<typename T>
bool Object<T>::isValid() const
{
  return _obj && _obj->type;
}

template<typename T>
Object<T>::operator bool() const
{
  return isValid();
}

template<typename T> Object<T>::operator Object<Empty>() const { return Object<Empty>(_obj);}
/// Check tha value actually has the T interface
template<typename T> void Object<T>::checkT()
{
  if (boost::is_same<T, Empty>::value || !_obj)
    return;

  const auto isMatchingType = [&] {
    return _obj->type->info() == typeOf<T>()->info()
      || _obj->type->inherits(typeOf<T>()) != ObjectTypeInterface::INHERITS_FAILED;
  };

  if (!isMatchingType())
  { // No T interface, try upgrading _obj
    detail::ProxyGeneratorMap& map = detail::proxyGeneratorMap();
    detail::ProxyGeneratorMap::iterator it = map.find(typeOf<T>()->info());
    if (it != map.end())
    {
      qiLogDebug("qitype.anyobject") << "Upgrading Object to specialized proxy.";
      AnyReference ref = it->second(AnyObject(_obj));
      _obj = ref.to<detail::ManagedObjectPtr>();
      ref.destroy();
      QI_ASSERT(isMatchingType());
      return;
    }
    throw std::runtime_error(std::string() + "Object does not have interface " + typeOf<T>()->infoString());
  }
}
template<typename T> ObjectUid Object<T>::uid() const
{
  QI_ASSERT(_obj);
  return _obj->uid;
}
template<typename T> T& Object<T>::asT() const
{
  const_cast<Object<T>* >(this)->checkT();
  return *static_cast<T*>(_obj->value);
}
template<typename T> T* Object<T>::operator->() const
{
  return &asT();
}
template<typename T> T& Object<T>::operator *() const
{
  return asT();
}
template<typename T> bool Object<T>::unique() const
{
  return _obj.unique();
}
template<typename T> GenericObject* Object<T>::asGenericObject() const
{
  return _obj.get();
}
template<typename T> void Object<T>::reset()
{
  _obj.reset();
}

namespace detail
{
  template<typename O>
  inline qi::FutureSync<SignalLink> GenericObjectBounce<O>::connect(unsigned int signal, AnyObject target, unsigned int slot) const
  {
    return go()->connect(signal, target, slot);
  }
}

/* Pretend that Object<T> is exactly shared_ptr<GenericObject>
 * Which it is in terms of memory layout.
 * But as a consequence, convert will happily create Object<T> for any T
 * without checking it.
 * Object<T> is handling this through the checkT() method.
 */
template<typename T>
class QI_API TypeImpl<Object<T>> :
  public TypeImpl<boost::shared_ptr<GenericObject>>
{
};

#ifdef _MSC_VER
/* Because we use types marked with QI_API and inheriting from Object<Empty>
 * (through AnyObject), then Object<Empty> functions must be explicitly
 * exported/imported to avoid link issues with MSVC.
 */
template class QI_API Object<Empty>;
#endif

}

namespace std
{
  /// The hash of an object is the hash of its ObjectUid.
  template<typename T>
  struct hash<qi::Object<T>>
  {
    std::size_t operator()(const qi::Object<T>& o) const
    {
      QI_ASSERT(o.asGenericObject());
      return hash<qi::ObjectUid>{}(o.asGenericObject()->uid);
    }
  };
} // namespace std

#endif  // _QITYPE_DETAIL_OBJECT_HXX_

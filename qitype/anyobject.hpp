#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_ANYOBJECT_HPP_
#define _QITYPE_ANYOBJECT_HPP_

#include <map>
#include <string>
#include <qi/atomic.hpp>
#include <qitype/api.hpp>
#include <qitype/manageable.hpp>
#include <qi/future.hpp>
#include <qitype/metasignal.hpp>
#include <qitype/metamethod.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/signal.hpp>
#include <qi/eventloop.hpp>
#include <qitype/typeobject.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#  pragma warning( disable: 4231 )
#endif

namespace qi {

  //all methods ID lesser than this constant are considered special.
  //they are reserved for internal use by qitype/qimessaging.
  //(see boundobject.cpp for details)
  static const unsigned int qiObjectSpecialMemberMaxUid = 100;

  /* We need shared typeid on Future<AnyReference>
   * If we do not export, typeids do not compare equals under some gcc-macos
   * Furthermore we get:
   * - macos: compiler warning and incorrect code if the template implementation is
   *   used before the extern declaration
   * - macos: compiler error if the extern is seen before a non-extern forced
   *  instantiation
   * - linux: linker error: if the symbol is marked hidden in some .o files,
   * and not hidden in others, hidden has precedence, and the extern prevents
   * the usage of the inlined code version.
   * - win32: if the whole template is exported, then no new instanciations
   * besides the one in the defining module can be created.
   */

  class SignalSubscriber;
  class SignalBase;

  class GenericObject;
  typedef boost::shared_ptr<GenericObject> AnyObject;
  typedef boost::weak_ptr<GenericObject>   ObjectWeakPtr;

  /* ObjectValue
  *  static version wrapping class C: Type<C>
  *  dynamic version: Type<DynamicObject>
  *
  * All the methods are convenience wrappers that bounce to the ObjectTypeInterface,
  * except Event Loop management
  * This class has pointer semantic. Do not use directly, use AnyObject,
  * obtained through Session, DynamicObjectBuilder or ObjectTypeBuilder.
  */
  class QITYPE_API GenericObject: public Manageable
  {
  public:
    GenericObject(ObjectTypeInterface *type, void *value);
    ~GenericObject();
    const MetaObject &metaObject();

#ifdef DOXYGEN
  // Help doxygen and the header reader a bit.
  template<typename R>
  qi::FutureSync<R> call(
                         const std::string& eventName,
                         qi::AutoAnyReference p1 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p2 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p3 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p4 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p5 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p6 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p7 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p8 = qi::AutoAnyReference());
  template<typename R>
  qi::FutureSync<R> call(
                         qi::MetaCallType callType,
                         const std::string& eventName,
                         qi::AutoAnyReference p1 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p2 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p3 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p4 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p5 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p6 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p7 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p8 = qi::AutoAnyReference());

  template<typename R>
  qi::FutureSync<R> async(
                         const std::string& eventName,
                         qi::AutoAnyReference p1 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p2 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p3 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p4 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p5 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p6 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p7 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p8 = qi::AutoAnyReference());
#else
    // Declare genCall, using overloads for all argument count instead of default values.
    #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma) \
    template<typename R> qi::FutureSync<R> call(       \
      const std::string& methodName comma              \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference));
    QI_GEN(genCall)
    #undef genCall
    #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma) \
    template<typename R> qi::FutureSync<R> async(     \
      const std::string& methodName comma              \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference));
    QI_GEN(genCall)
    #undef genCall
    #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma) \
    template<typename R> qi::FutureSync<R> call(     \
      qi::MetaCallType callType,                         \
      const std::string& methodName comma              \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference));
    QI_GEN(genCall)
    #undef genCall
#endif // DOXYGEN

    qi::Future<AnyReference> metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    /** Find method named \p named callable with arguments \p parameters
    */
    unsigned int findMethod(const std::string& name, const GenericFunctionParameters& parameters);
    /** Resolve the method Id and bounces to metaCall
    * @param signature method name or method signature 'name::(args)'
    *        if signature is given, an exact match is required
    */
    qi::Future<AnyReference> metaCall(const std::string &nameWithOptionalSignature, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);

    void post(const std::string& eventName,
                   qi::AutoAnyReference p1 = qi::AutoAnyReference(),
                   qi::AutoAnyReference p2 = qi::AutoAnyReference(),
                   qi::AutoAnyReference p3 = qi::AutoAnyReference(),
                   qi::AutoAnyReference p4 = qi::AutoAnyReference(),
                   qi::AutoAnyReference p5 = qi::AutoAnyReference(),
                   qi::AutoAnyReference p6 = qi::AutoAnyReference(),
                   qi::AutoAnyReference p7 = qi::AutoAnyReference(),
                   qi::AutoAnyReference p8 = qi::AutoAnyReference());

    void metaPost(unsigned int event, const GenericFunctionParameters& params);
    void metaPost(const std::string &nameWithOptionalSignature, const GenericFunctionParameters &in);

    /** Connect an event to an arbitrary callback.
     *
     * If you are within a service, it is recommended that you connect the
     * event to one of your Slots instead of using this method.
     */
    template <typename FUNCTOR_TYPE>
    qi::FutureSync<SignalLink> connect(const std::string& eventName, FUNCTOR_TYPE callback,
                         MetaCallType threadingModel = MetaCallType_Direct);


    qi::FutureSync<SignalLink> connect(const std::string &name, const SignalSubscriber& functor);

    /// Calls given functor when event is fired. Takes ownership of functor.
    qi::FutureSync<SignalLink> connect(unsigned int signal, const SignalSubscriber& subscriber);

    /** Connect an event to a method.
     * Recommended use is when target is not a proxy.
     * If target is a proxy and this is server-side, the event will be
     *    registered localy and the call will be forwarded.
     * If target and this are proxies, the message will be routed through
     * the current process.
     */
    qi::FutureSync<SignalLink> connect(unsigned int signal, qi::AnyObject target, unsigned int slot);

    /// Disconnect an event link. Returns if disconnection was successful.
    qi::FutureSync<void> disconnect(SignalLink linkId);

    template<typename T>
    qi::FutureSync<T> property(const std::string& name);

    template<typename T>
    qi::FutureSync<void> setProperty(const std::string& name, const T& val);

    //Low Level Properties
    qi::FutureSync<AnyValue> property(unsigned int id);
    qi::FutureSync<void> setProperty(unsigned int id, const AnyValue &val);


    //bool isValid() { return type && value;}
    ObjectTypeInterface*  type;
    void*        value;
  };
#ifdef DOXYGEN
  /** Perform an asynchronous call on a method.
   * @param instance a pointer or shared-pointer to an instance of a class known to type system.
  */
  template<typename R, typename T>
  qi::FutureSync<R> async(
                         T instancePointerOrSharedPointer,
                         const std::string& methodName,
                         qi::AutoAnyReference p1 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p2 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p3 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p4 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p5 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p6 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p7 = qi::AutoAnyReference(),
                         qi::AutoAnyReference p8 = qi::AutoAnyReference());
#else
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)         \
    template<typename R,typename T> qi::FutureSync<R> async(   \
      T* instance,                                                 \
      const std::string& methodName comma                         \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference));       \
    template<typename R,typename T> qi::FutureSync<R> async(   \
      boost::shared_ptr<T> instance,                              \
      const std::string& methodName comma                         \
      QI_GEN_ARGSDECLSAMETYPE(n, qi::AutoAnyReference));
    QI_GEN(genCall)
    #undef genCall
#endif
   // C4251
  template <typename FUNCTION_TYPE>
  qi::FutureSync<SignalLink> GenericObject::connect(const std::string& eventName,
                                                      FUNCTION_TYPE callback,
                                                      MetaCallType model)
  {
    return connect(eventName,
      SignalSubscriber(AnyFunction::from(callback), model));
  }


  /** Make a call honoring ThreadingModel requirements
   *
   * Check the following rules in order:
   * - If \p el is set, force call in it overriding all rules.
   * - If method type is not auto, honor it, overriding callType
   * - If callType is set (not auto), honor it.
   * - Be synchronous.
   *
   * When the call is finally made, if ObjectThreadingModel
   * is SingleThread, acquire the object lock.
  */
  QITYPE_API qi::Future<AnyReference> metaCall(EventLoop* el,
    ObjectThreadingModel objectThreadingModel,
    MetaCallType methodThreadingModel,
    MetaCallType callType,
    Manageable* manageable,
    unsigned int methodId,
    AnyFunction func, const GenericFunctionParameters& params, bool noCloneFirst=false);

  class QITYPE_API Proxy
  {
  public:
    Proxy(qi::AnyObject obj) : _obj(obj) {}
    qi::AnyObject asObject() { return _obj;}
  protected:
    qi::AnyObject _obj;
  };

  class TypeProxy: public ObjectTypeInterface
  {
  public:
    virtual const MetaObject& metaObject(void* instance)
    {
      Proxy* ptr = static_cast<Proxy*>(instance);
      return ptr->asObject()->metaObject();
    }
    virtual qi::Future<AnyReference> metaCall(void* instance, Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto)
    {
      Proxy* ptr = static_cast<Proxy*>(instance);
      return ptr->asObject()->metaCall(method, params, callType);
    }
    virtual void metaPost(void* instance, Manageable* context, unsigned int signal, const GenericFunctionParameters& params)
    {
      Proxy* ptr = static_cast<Proxy*>(instance);
      ptr->asObject()->metaPost(signal, params);
    }
    virtual qi::Future<SignalLink> connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber)
    {
      Proxy* ptr = static_cast<Proxy*>(instance);
      return ptr->asObject()->connect(event, subscriber);
    }
    virtual qi::Future<void> disconnect(void* instance, Manageable* context, SignalLink linkId)
    {
       Proxy* ptr = static_cast<Proxy*>(instance);
       return ptr->asObject()->disconnect(linkId);
    }
    virtual const std::vector<std::pair<TypeInterface*, int> >& parentTypes()
    {
      static std::vector<std::pair<TypeInterface*, int> > empty;
      return empty;
    }
    virtual qi::Future<AnyValue> property(void* instance, unsigned int id)
    {
      Proxy* ptr = static_cast<Proxy*>(instance);
      AnyObject obj = ptr->asObject();
      return obj->type->property(obj->value, id);
    }
    virtual qi::Future<void> setProperty(void* instance, unsigned int id, AnyValue value)
    {
      Proxy* ptr = static_cast<Proxy*>(instance);
      AnyObject obj = ptr->asObject();
      return obj->type->setProperty(obj->value, id, value);
    }

  };
#define QI_TYPE_PROXY(name)                            \
  namespace qi {                                       \
    template<> class TypeImpl<name>                    \
      : public TypeProxy {                             \
        typedef DefaultTypeImplMethods<name> Methods;  \
        _QI_BOUNCE_TYPE_METHODS(Methods);              \
    };                                                 \
    namespace detail {                                 \
    template<> struct TypeManager<name>                \
      : public TypeManagerNonDefaultConstructible<name> {}; \
     }                                                 \
  }

  /** Register \p Proxy as a proxy class.
   * Required for bound methods to accept a ProxyPtr as argument
   * Proxy must be constructible with an AnyObject as argument
   * @return unused value, present to ease registration at static initialisation
   */
  template<typename Proxy>
  bool registerProxy();

  /** Register \p Proxy as a proxy class for interface \p Interface.
   * Required for bound methods to accept a InterfacePtr as argument
   * Proxy must be constructible with an AnyObject as argument
   * @return unused value, present to ease registration at static initialisation
   */
  template<typename Proxy, typename Interface>
  bool registerProxyInterface();

  #define QI_REGISTER_PROXY(Proxy) \
  namespace {                      \
    static bool BOOST_PP_CAT(_qi_register_proxy_, Proxy) = ::qi::registerProxy<Proxy>(); \
  }

  #define QI_REGISTER_PROXY_INTERFACE(Proxy, Interface) \
  namespace {                      \
    static bool BOOST_PP_CAT(_qi_register_proxy_, Proxy) = ::qi::registerProxyInterface<Proxy, Interface>(); \
  }
}

#define __QI_REGISTER_ELEMENT(_, name, field) \
  b.advertise(BOOST_PP_STRINGIZE(field), & name::field); // do not remove the space

/** Register an object to the typesystem
 * @param name the class name, without any namespace
 * @param ARGS the names of the methods, signals and properties of the class
 *
 * @warning must be called from an unique compilation unit (not a header), from
 * within the namespace of the class
 */
#define QI_REGISTER_OBJECT(name, ...)                                             \
static bool _qiregister##name() {                                              \
   ::qi::ObjectTypeBuilder<name > b;                                            \
   QI_VAARGS_APPLY(__QI_REGISTER_ELEMENT, name, __VA_ARGS__)                   \
   b.registerType();                                                           \
   return true;                                                                \
 }                                                                             \
 static bool BOOST_PP_CAT(__qi_registration, __LINE__) = _qiregister##name();

/** Similar to QI_REGISTER_OBJECT() but also registering inheritance from an
 * other registered class
 */
#define QI_REGISTER_CHILD_OBJECT(parent, name, ...)                                             \
static bool _qiregister##name() {                                              \
   ::qi::ObjectTypeBuilder<name > b;    \
   b.inherits<parent>();                \
   QI_VAARGS_APPLY(__QI_REGISTER_ELEMENT, name, __VA_ARGS__)                   \
   b.registerType();                                                           \
   return true;                                                                \
 }                                                                             \
 static bool BOOST_PP_CAT(__qi_registration, __LINE__) = _qiregister##name();

/** Register name as a template object type
 * Remaining arguments are the methods, signals and properties of the object.
 * Use QI_TEMPLATE_TYPE_GET() to access the TemplateTypeInterface from a Type.
 */
#define QI_REGISTER_TEMPLATE_OBJECT(name, ...)                                                         \
  namespace qi {                                                                                       \
    template<typename T> class TypeOfTemplateImpl<name, T>: public TypeOfTemplateDefaultImpl<name, T>  \
    {                                                                                                  \
    public:                                                                                            \
      TypeOfTemplateImpl(): _next(0) {}                                                                \
      virtual TypeInterface* next()                                                                             \
      {                                                                                                \
        if (!_next)                                                                                    \
        {                                                                                              \
           ObjectTypeBuilder<name<T> > b(false);                                               \
           QI_VAARGS_APPLY(__QI_REGISTER_ELEMENT, name<T> , __VA_ARGS__)                                \
           _next = b.type();                                                                             \
        }                                                                                              \
        return _next;                                                                                  \
      }                                                                                                \
      TypeInterface* _next;                                                                                     \
    };                                                                                                 \
  } \
  QI_TEMPLATE_TYPE_DECLARE(name)



#include <qitype/details/anyobject.hxx>

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_ANYOBJECT_HPP_

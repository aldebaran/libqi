#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_OBJECTTYPEBUILDER_HPP_
#define _QI_TYPE_OBJECTTYPEBUILDER_HPP_

#include <qi/api.hpp>
#include <string>

#include <boost/function.hpp>
#include <qi/signature.hpp>
#include <sstream>
#include <qi/type/typeinterface.hpp>
#include <qi/type/detail/staticobjecttype.hpp>
#include <qi/type/detail/accessor.hxx>
#include <qi/type/detail/object.hxx>
#include <qi/property.hpp>

namespace qi {

namespace detail {
  struct ObjectTypeData;
}

  class MetaObject;

  class SignalBase;
  class ObjectTypeInterface;
  class TypeInterface;
  template<typename T> class SignalF;
  class ObjectTypeBuilderPrivate;

  class QI_API ObjectTypeBuilderBase
  {
  public:
    ObjectTypeBuilderBase();
    ~ObjectTypeBuilderBase();

    using SignalMemberGetter = boost::function<SignalBase* (void*)>;
    using PropertyMemberGetter = boost::function<PropertyBase* (void*)>;

    /// Sets a description for the type to build.
    void setDescription(const std::string& description);

    // input: template-based

    /// Declare the class type for which this StaticBuilder is.
    template<typename T> void  buildFor(bool autoRegister = true);

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function, MetaCallType threadingModel = MetaCallType_Auto, int id = -1);

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(MetaMethodBuilder& name, FUNCTION_TYPE function, MetaCallType threadingModel = MetaCallType_Auto, int id = -1);

    template<typename A>
    unsigned int advertiseSignal(const std::string& eventName, A accessor, int id = -1);

    template <typename T>
    inline unsigned int advertiseSignal(const std::string& name, SignalMemberGetter getter, int id = -1);

    template <typename A>
    inline unsigned int advertiseProperty(const std::string& propertyName, A accessor);

    template<typename T>
    inline unsigned int advertiseProperty(const std::string& eventName, PropertyMemberGetter getter);

    /** create a T, wrap in a AnyObject
     *  All template parameters are given to the T constructor except the first one
     */
    template<typename T, typename... Args>
    inline unsigned int advertiseFactory(const std::string& name)
    {
      return advertiseMethod(name, &constructObject<T, Args...>);
    }

    template<typename P>
    void inherits(int offset);

    // Advertise anything, dispatch on {method, event, property} based on T.
    template<typename T>
    ObjectTypeBuilderBase& advertise(const std::string& name, T element);
    // Like advertise, but return the id
    template<typename T>
    unsigned int advertiseId(const std::string& name, T element);
    // input: type-erased

    unsigned int xAdvertiseMethod(MetaMethodBuilder& builder, AnyFunction func, MetaCallType threadingModel = MetaCallType_Auto, int id = -1);
    unsigned int xAdvertiseSignal(const std::string &name, const qi::Signature& signature, SignalMemberGetter getter, int id = -1);
    unsigned int xAdvertiseProperty(const std::string& name, const qi::Signature& signature, PropertyMemberGetter getter, int id = -1);
    void xBuildFor(TypeInterface* type, bool autoRegister, qi::AnyFunction strandAccessor);
    void inherits(TypeInterface* parentType, int offset);

    // Configuration

    void setThreadingModel(ObjectThreadingModel model);

    // output
    const MetaObject& metaObject();
    AnyObject object(void* ptr, boost::function<void (GenericObject*)> onDestroy = boost::function<void (GenericObject*)>());
    ObjectTypeInterface* type();

    /// Register type to typeof. Called by type()
    inline virtual void registerType() {};

    const detail::ObjectTypeData& typeData();
  private:
    ObjectTypeBuilderPrivate* _p;
  };

  template<typename T>
  class ObjectTypeBuilder : public ObjectTypeBuilderBase
  {
  public:
    ObjectTypeBuilder(bool autoRegister=true)
    {
      buildFor<T>(autoRegister);
    }

    /** Declare that T inherits from U.
     *
     * @warning If type \p U has registered methods,signals, properties,
     * then inherits() must be called before registering anything on type T.
     */
    template<typename U> void inherits();

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name,
                                        FUNCTION_TYPE function,
                                        MetaCallType threadingModel = MetaCallType_Auto,
                                        int id = -1);

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(MetaMethodBuilder& name,
                                        FUNCTION_TYPE function,
                                        MetaCallType threadingModel = MetaCallType_Auto,
                                        int id = -1);


    /// Register type to typeOf<T>, to avoid both TypeImpl<T> and type() being present
    inline virtual void registerType();

    inline AnyObject object(T* ptr, boost::function<void (GenericObject*)> onDestroy = boost::function<void (GenericObject*)>());
  };
}

#include <qi/type/detail/objecttypebuilder.hxx>

/**
 * Advertise a member on a builder.
 * \param builder The builder on which to advertise
 * \param cls The class to register
 * \param name The name of the member
 */
#define QI_OBJECT_BUILDER_ADVERTISE(builder, cls, name) \
  builder.advertise(BOOST_PP_STRINGIZE(name), &cls::name)
/**
 * Advertise an overloaded function on a builder.
 * \param builder The builder on which to advertise
 * \param cls The class to register
 * \param name The name of the function
 * \param ret The return type
 * \param args The arguments as a tuple (ex: (const std::string&))
 *
 * You must use this macro with the exact types of your method with references,
 * pointers, cv-qualifiers (to the arguments and the method).
 */
#define QI_OBJECT_BUILDER_ADVERTISE_OVERLOAD(builder, cls, name, ret, args) \
  builder.advertiseMethod(BOOST_PP_STRINGIZE(name), static_cast<ret (cls::*)args>(&cls::name))

#define __QI_REGISTER_ELEMENT(_, name, field) \
  b.advertise(BOOST_PP_STRINGIZE(field), & name::field); // do not remove the space

/** Register an object to the typesystem
 * @param name the class name, without any namespace
 * @param ... the names of the methods, signals and properties of the class
 *
 * @warning must be called from an unique compilation unit (not a header), from
 * within the namespace of the class
 */
#define QI_REGISTER_OBJECT(name, ...)                         \
  static bool _qiregister##name() {                           \
    ::qi::ObjectTypeBuilder<name > b;                         \
    QI_VAARGS_APPLY(__QI_REGISTER_ELEMENT, name, __VA_ARGS__) \
    b.registerType();                                         \
    return true;                                              \
  }                                                           \
  static bool BOOST_PP_CAT(__qi_registration, __LINE__) = _qiregister##name();

#define QI_REGISTER_MT_OBJECT(name, ...)                       \
  static bool _qiregister##name() {                            \
    ::qi::ObjectTypeBuilder<name > b;                          \
    b.setThreadingModel(qi::ObjectThreadingModel_MultiThread); \
    QI_VAARGS_APPLY(__QI_REGISTER_ELEMENT, name, __VA_ARGS__)  \
    b.registerType();                                          \
    return true;                                               \
  }                                                            \
  static bool BOOST_PP_CAT(__qi_registration, __LINE__) = _qiregister##name();

/** Register object \p name as implementation of \p parent
 * FIXME: support inheritance with offset.
 * This implementation just bounces to parent's TypeInterfac.
 * If that doesn't fit your need, you can always re-register
 * everything from the interface on your class.
 */
#define QI_REGISTER_IMPLEMENTATION(parent, name)                                                           \
  static bool BOOST_PP_CAT(__qi_registration_func, __LINE__)()                                             \
  {                                                                                                        \
    qi::detail::ForceProxyInclusion<parent>().dummyCall();                                                 \
    qi::registerType(typeid(name), qi::typeOf<parent>());                                                  \
    name* ptr = static_cast<name*>(reinterpret_cast<void*>(0x10000));                                      \
    parent* pptr = ptr;                                                                                    \
    intptr_t offset = reinterpret_cast<intptr_t>(pptr) - reinterpret_cast<intptr_t>(ptr);                  \
    if (offset)                                                                                            \
    {                                                                                                      \
      qiLogError("qitype.register") << "non-zero offset for implementation " << #name << " of " << #parent \
                                    << ", call will fail at runtime";                                      \
      throw std::runtime_error("non-zero offset between implementation and interface");                    \
    }                                                                                                      \
    return true;                                                                                           \
  }                                                                                                        \
  static bool BOOST_PP_CAT(__qi_registration, __LINE__) = BOOST_PP_CAT(__qi_registration_func, __LINE__)();

#define _QI_REGISTER_TEMPLATE_OBJECT(name, model, ...)                             \
  namespace qi                                                                     \
  {                                                                                \
  template <>                                                                      \
  class QI_API TypeOfTemplate<name> : public detail::StaticObjectTypeBase          \
  {                                                                                \
  public:                                                                          \
    virtual TypeInterface* templateArgument() = 0;                                 \
  };                                                                               \
  template <typename T>                                                            \
  class TypeOfTemplateImpl<name, T> : public TypeOfTemplate<name>                  \
  {                                                                                \
  public:                                                                          \
    TypeOfTemplateImpl()                                                           \
    {                                                                              \
      /* early self registering to avoid recursive init */                         \
      ::qi::registerType(typeid(name<T>), this);                                   \
      ObjectTypeBuilder<name<T> > b(false);                                        \
      b.setThreadingModel(model);                                                  \
      QI_VAARGS_APPLY(__QI_REGISTER_ELEMENT, name<T>, __VA_ARGS__)                 \
      this->initialize(b.metaObject(), b.typeData());                              \
    }                                                                              \
    TypeInterface* templateArgument() override { return typeOf<T>(); }             \
    using Methods = DefaultTypeImplMethods<name<T>>;                               \
    _QI_BOUNCE_TYPE_METHODS(Methods);                                              \
  };                                                                               \
  }                                                                                \
  QI_TEMPLATE_TYPE_DECLARE(name)

/** Register name as a template object type
 * Remaining arguments are the methods, signals and properties of the object.
 * Use QI_TEMPLATE_TYPE_GET() to access the TypeOfTemplate<T> from a Type.
 */
#define QI_TEMPLATE_OBJECT(name, ...)                                   \
  _QI_REGISTER_TEMPLATE_OBJECT(name, ObjectThreadingModel_SingleThread, \
                               __VA_ARGS__)

/** \deprecated since 2.3, use QI_TEMPLATE_OBJECT
 */
#define QI_REGISTER_TEMPLATE_OBJECT(name, ...)                           \
  QI_TEMPLATE_OBJECT(name, __VA_ARGS__)

/** Same as QI_TEMPLATE_OBJECT for multithread objects
 */
#define QI_MT_TEMPLATE_OBJECT(name, ...)                               \
  _QI_REGISTER_TEMPLATE_OBJECT(name, ObjectThreadingModel_MultiThread, \
                               __VA_ARGS__)

// sometimes we need to use type-erased futures, but only those two methods are needed, so register our futures as
// normal objects with these methods
namespace qi
{
template <>
class QI_API TypeOfTemplate<qi::Future> : public detail::StaticObjectTypeBase
{
public:
  virtual TypeInterface* templateArgument() = 0;
};
template <>
class QI_API TypeOfTemplate<qi::FutureSync> : public detail::StaticObjectTypeBase
{
public:
  virtual TypeInterface* templateArgument() = 0;
};
template <template<typename> class FutT, typename T>
class TypeOfTemplateFutImpl : public TypeOfTemplate<FutT>
{
public:
  TypeOfTemplateFutImpl()
  {
    /* early self registering to avoid recursive init */
    ::qi::registerType(typeid(FutT<T>), this);
    ObjectTypeBuilder<FutT<T> > b(false);
    b.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
#define ADVERTISE(meth) \
    b.advertiseMethod(#meth, &FutT<T>::meth)
    ADVERTISE(_connect);
    ADVERTISE(error);
    ADVERTISE(hasError);
    ADVERTISE(isCanceled);
    ADVERTISE(cancel);
    ADVERTISE(value);
    ADVERTISE(waitUntil);
    ADVERTISE(waitFor);
    ADVERTISE(isRunning);
    ADVERTISE(isFinished);
    ADVERTISE(isValid);
#undef ADVERTISE
    // this method is useful to get a future<anyvalue> through a simple async call
    // it is used in libqi-python
    b.advertiseMethod("_getSelf",
                      static_cast<qi::Future<T>(*)(FutT<T>*)>(
                          [](FutT<T>* fut) -> qi::Future<T> {
                            return *fut;
                          }));
    this->initialize(b.metaObject(), b.typeData());
  }
  TypeInterface* templateArgument() override
  {
    return typeOf<T>();
  }
  using Methods = DefaultTypeImplMethods<FutT<T>>;
  _QI_BOUNCE_TYPE_METHODS(Methods);
};
template <typename T>
class TypeOfTemplateImpl<qi::Future, T> : public TypeOfTemplateFutImpl<qi::Future, T> {};
template <typename T>
class TypeOfTemplateImpl<qi::FutureSync, T> : public TypeOfTemplateFutImpl<qi::FutureSync, T> {};
}
QI_TEMPLATE_TYPE_DECLARE(qi::Future)
QI_TEMPLATE_TYPE_DECLARE(qi::FutureSync)

QI_MT_TEMPLATE_OBJECT(qi::Promise, setValue, setError, setCanceled, future, value, trigger);

namespace qi { namespace detail {
  template<typename T> struct TypeManager<Future<T> >: public TypeManagerDefaultStruct<Future<T> > {};
  template<typename T> struct TypeManager<FutureSync<T> >: public TypeManagerDefaultStruct<FutureSync<T> > {};
  template<typename T> struct TypeManager<Promise<T> >: public TypeManagerDefaultStruct<Promise<T> > {};
}}

#endif  // _QITYPE_OBJECTTYPEBUILDER_HPP_

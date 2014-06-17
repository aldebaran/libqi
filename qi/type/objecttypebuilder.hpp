#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_OBJECTTYPEBUILDER_HPP_
#define _QI_TYPE_OBJECTTYPEBUILDER_HPP_

#include <qi/type/api.hpp>
#include <string>

#include <boost/function.hpp>
#include <qi/signature.hpp>
#include <sstream>
#include <qi/type/typeinterface.hpp>
#include <qi/type/details/accessor.hxx>
#include <qi/anyobject.hpp>
#include <qi/property.hpp>

namespace qi {

  class MetaObject;

  class SignalBase;
  class ObjectTypeInterface;
  class TypeInterface;
  template<typename T> class SignalF;
  class ObjectTypeBuilderPrivate;
  struct ObjectTypeData;
  class QITYPE_API ObjectTypeBuilderBase
  {
  public:
    ObjectTypeBuilderBase();
    ~ObjectTypeBuilderBase();

    typedef boost::function<SignalBase* (void*)> SignalMemberGetter;
    typedef boost::function<PropertyBase* (void*)> PropertyMemberGetter;

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
    void xBuildFor(TypeInterface* type, bool autoRegister = true);
    void inherits(TypeInterface* parentType, int offset);

    // Configuration

    void setThreadingModel(ObjectThreadingModel model);

    // output
    const MetaObject& metaObject();
    AnyObject object(void* ptr, boost::function<void (GenericObject*)> onDestroy = boost::function<void (GenericObject*)>());
    ObjectTypeInterface* type();

    /// Register type to typeof. Called by type()
    inline virtual void registerType() {};

    const ObjectTypeData& typeData();
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

#include <qi/type/details/objecttypebuilder.hxx>

#define __QI_REGISTER_ELEMENT(_, name, field) \
  b.advertise(BOOST_PP_STRINGIZE(field), & name::field); // do not remove the space

/** Register an object to the typesystem
 * @param name the class name, without any namespace
 * @param ARGS the names of the methods, signals and properties of the class
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
#define QI_REGISTER_IMPLEMENTATION(parent, name)                             \
  static bool _qiregister##name() {                                          \
    qi::registerType(typeid(name), qi::typeOf<parent>());                    \
    name* ptr = (name*)(void*)0x10000;                                       \
    parent* pptr = ptr;                                                      \
    int offset = (intptr_t)(void*)pptr - (intptr_t)(void*) ptr;              \
    if (offset)                                                              \
      qiLogError("qitype.register") << "non-zero offset for implementation " \
        << #name <<" of " << #parent << ", call will fail at runtime";       \
    return true;                                                             \
  }                                                                          \
  static bool BOOST_PP_CAT(__qi_registration, __LINE__) = _qiregister##name();

/** Register name as a template object type
 * Remaining arguments are the methods, signals and properties of the object.
 * Use QI_TEMPLATE_TYPE_GET() to access the TemplateTypeInterface from a Type.
 */
#define QI_REGISTER_TEMPLATE_OBJECT(name, ...)                           \
  namespace qi {                                                         \
    template<typename T> class TypeOfTemplateImpl<name, T> :             \
        public TypeOfTemplateDefaultImpl<name, T>                        \
    {                                                                    \
    public:                                                              \
      TypeOfTemplateImpl(): _next(0) {}                                  \
      virtual TypeInterface* next()                                      \
      {                                                                  \
        if (!_next)                                                      \
        {                                                                \
           ObjectTypeBuilder<name<T> > b(false);                         \
           QI_VAARGS_APPLY(__QI_REGISTER_ELEMENT, name<T> , __VA_ARGS__) \
           _next = b.type();                                             \
        }                                                                \
        return _next;                                                    \
      }                                                                  \
      TypeInterface* _next;                                              \
    };                                                                   \
  }                                                                      \
  QI_TEMPLATE_TYPE_DECLARE(name)

// FIXME move this, objecttypebuilder might not be included
QI_REGISTER_TEMPLATE_OBJECT(qi::Future    , _connect, isFinished, value, wait, isRunning, isCanceled, hasError, error);
QI_REGISTER_TEMPLATE_OBJECT(qi::FutureSync, _connect, isFinished, value, wait, isRunning, isCanceled, hasError, error, async);
QI_REGISTER_TEMPLATE_OBJECT(qi::Promise, setValue, setError, setCanceled, reset, future, value, trigger);
namespace qi { namespace detail {
  template<typename T> struct TypeManager<Future<T> >: public TypeManagerDefaultStruct<Future<T> > {};
  template<typename T> struct TypeManager<FutureSync<T> >: public TypeManagerDefaultStruct<FutureSync<T> > {};
  template<typename T> struct TypeManager<Promise<T> >: public TypeManagerDefaultStruct<Promise<T> > {};
}}

#endif  // _QITYPE_OBJECTTYPEBUILDER_HPP_

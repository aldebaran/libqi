/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  METAOBJECTBUILDER_HPP_
# define METAOBJECTBUILDER_HPP_

#include <qimessaging/api.hpp>
#include <string>

#include <boost/function.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/metafunction.hpp>
#include <sstream>
#include <qimessaging/type.hpp>
#include <qimessaging/genericobject.hpp>

#include <qimessaging/staticobjecttype.hpp>
namespace qi {

  class MetaObject;

  class SignalBase;
  class ObjectType;
  class Type;
  template<typename T> class Signal;
  class StaticObjectBuilderPrivate;
  class DynamicObjectBuilderPrivate;

  class QIMESSAGING_API DynamicObjectBuilder
  {
  public:
    DynamicObjectBuilder();
    ~DynamicObjectBuilder();

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method);
    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function);
    template<typename FUNCTION_TYPE>
    inline unsigned int advertiseEvent(const std::string& eventName);


    int xAdvertiseMethod(const std::string &retsig, const std::string& signature, MetaCallable func);
    int xAdvertiseEvent(const std::string& signature);

    qi::GenericObject object();
  public:
    DynamicObjectBuilderPrivate *_p;
    QI_DISALLOW_COPY_AND_ASSIGN(DynamicObjectBuilder);
  };

  class QIMESSAGING_API StaticObjectBuilder
  {
  public:
    StaticObjectBuilder();
    ~StaticObjectBuilder();

    typedef boost::function<SignalBase* (void*)> SignalMemberGetter;

    // input: template-based

    /// Declare the class type for which this StaticBuilder is.
    template<typename T> void  buildFor();
    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function);
    template <typename C, typename T>
    inline unsigned int advertiseEvent(const std::string& eventName, Signal<T> C::* signalAccessor);
    template <typename T>
    inline unsigned int advertiseEvent(const std::string& name, SignalMemberGetter getter);

    // input: type-erased

    int xAdvertiseMethod(const std::string &retsig, const std::string& signature, GenericMethod func);
    int xAdvertiseEvent(const std::string& signature, SignalMemberGetter getter);
    void xBuildFor(Type* type);

    // output
    const MetaObject& metaObject();
    template<typename T> GenericObject makeObject(T* ptr);
    template<typename T> ObjectType* type();

  private:
    ObjectType* &_type();
    void _init();
    StaticObjectBuilderPrivate* _p;
  };

  template<typename T> void StaticObjectBuilder::buildFor()
  {
    xBuildFor(typeOf<T>());
  }

  template <typename FUNCTION_TYPE>
  unsigned int StaticObjectBuilder::advertiseMethod(const std::string& name, FUNCTION_TYPE function)
  {
    // FIXME validate type
    return xAdvertiseMethod(detail::FunctionSignature<FUNCTION_TYPE>::sigreturn(),
      name + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature(),
      makeGenericMethod(function));
  }

  template <typename FUNCTION_TYPE>
  unsigned int DynamicObjectBuilder::advertiseMethod(const std::string& name, FUNCTION_TYPE function)
  {
    // FIXME validate type
    return xAdvertiseMethod(detail::FunctionSignature<FUNCTION_TYPE>::sigreturn(),
      name + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature(),
      makeCallable(function));
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int DynamicObjectBuilder::advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method)
  {
    return xAdvertiseMethod(detail::FunctionSignature<METHOD_TYPE >::sigreturn(),
      name + "::" + detail::FunctionSignature<METHOD_TYPE >::signature(),
      makeCallable(makeGenericFunction(object, method)));
  }

  template <typename C, typename T>
  SignalBase signalAccess(Signal<T> C::* ptr, void* instance)
  {
    C* c = reinterpret_cast<C*>(instance);
    return (*c).*ptr;
  }

  template <typename C, typename T>
  unsigned int StaticObjectBuilder::advertiseEvent(const std::string& eventName, Signal<T> C::* signalAccessor)
  {
    // FIXME validate type
    SignalMemberGetter fun = boost::bind(&signalAccess<C, T>, signalAccessor, _1);
    return xAdvertiseEvent(eventName + "::" + detail::FunctionSignature<T>::signature(), fun);
  }

  template <typename T> unsigned int StaticObjectBuilder::advertiseEvent(const std::string& name, SignalMemberGetter getter)
  {
    return xAdvertiseEvent(name + "::" + detail::FunctionSignature<T>::signature(), getter);
  }

  template <typename T> unsigned int DynamicObjectBuilder::advertiseEvent(const std::string& name)
  {
    return xAdvertiseEvent(name + "::" + detail::FunctionSignature<T>::signature());
  }

  template<typename T> GenericObject StaticObjectBuilder::makeObject(T* ptr)
  {
    // FIXME validate type
    GenericObject o;
    o.type = type<T>();
    T** nptr = new T*;
    *nptr = ptr;
    o.value = nptr;
    return o;
  }

  template<typename T> ObjectType* StaticObjectBuilder::type()
  {
    ObjectType* &t = _type();
    if (!t)
    {
      t = new StaticObject<T>();
      _init();
    }
    return t;

  }

}

#endif /* !METAOBJECTBUILDER_PP_ */

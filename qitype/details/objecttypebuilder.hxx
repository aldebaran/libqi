#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_OBJECTTYPEBUILDER_HXX_
#define _QITYPE_DETAILS_OBJECTTYPEBUILDER_HXX_

#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/mpl/front.hpp>
#include <qitype/objecttypebuilder.hpp>
#include <qitype/metamethod.hpp>

namespace qi {


  template<typename T> void ObjectTypeBuilderBase::buildFor()
  {
    // We are erasing T here: we must pass everything the builder need to know about t:
    // - typeid
    // - cloner/deleter
    // - serializer, ...
    // => wee need all Type* methods, but we do not want another Type*
    // to anwser to typeOf<T>
    xBuildFor(new DefaultTypeImpl<T>());
  }

  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilderBase::advertiseMethod(const std::string& name,
                                                      FUNCTION_TYPE function,
                                                      MetaCallType threadingModel,
                                                      int id)
  {
    MetaMethodBuilder builder;
    GenericFunction f = makeGenericFunction(function);
    if (! boost::is_member_function_pointer<FUNCTION_TYPE>::value)
      f.dropFirstArgument();
    builder.setSignatures(name, f);
    return xAdvertiseMethod(builder, f, threadingModel, id);
  }

  template<typename U>
  void ObjectTypeBuilderBase::inherits(int offset)
  {
    return inherits(typeOf<
      typename boost::remove_reference<U>::type>(), offset);
  }

  template<typename T>
  template<typename U>
  void ObjectTypeBuilder<T>::inherits()
  {
    qiLogCategory("qitype.objectbuilder");
    T* ptr = (T*)(void*)0x10000;
    U* pptr = ptr;
    int offset = (long)(void*)pptr - (long)(void*) ptr;
    qiLogDebug() << "Offset check " << pptr <<" " << ptr << " " << offset;
    qiLogDebug() << typeid(ptr).name() << " " << typeid(pptr).name();
    return ObjectTypeBuilderBase::inherits<U>(offset);
  }

  namespace detail
  {
    template<typename F, typename T> void checkRegisterParent(
      ObjectTypeBuilder<T>& , boost::false_type) {}
    template<typename F, typename T> void checkRegisterParent(
      ObjectTypeBuilder<T>& builder, boost::true_type)
    {
      typedef typename boost::function_types::parameter_types<F>::type ArgsType;
      typedef typename boost::mpl::front<ArgsType>::type DecoratedClassType;
      typedef typename boost::remove_reference<DecoratedClassType>::type ClassType;
      builder.template inherits<ClassType>();
    }
  };

  template <typename T>
  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilder<T>::advertiseMethod(const std::string& name, FUNCTION_TYPE function, MetaCallType threadingModel, int id)
  {
    // Intercept advertise to auto-register parent type if this is a parent method
    // Note: if FUNCTION_TYPE is a grandparent method, we will incorrectly add it
    // as a child
    detail::checkRegisterParent<FUNCTION_TYPE>(
      *this,
      typename boost::function_types::is_member_function_pointer<FUNCTION_TYPE >::type());
    return ObjectTypeBuilderBase::advertiseMethod(name, function, threadingModel, id);
  }

  template <typename T>
  ObjectPtr ObjectTypeBuilder<T>::object(T* ptr, boost::function<void (GenericObject*)> onDestroy)
  {
    return ObjectTypeBuilderBase::object(static_cast<void*>(ptr), onDestroy);
  }


  template<typename T>
  void ObjectTypeBuilder<T>::registerType()
  {
    ::qi::registerType(typeid(T), type());
  }

  template <typename C, typename T>
  SignalBase* signalAccess(Signal<T> C::* ptr, void* instance)
  {
    C* c = reinterpret_cast<C*>(instance);
    return &((*c).*ptr);
  }

  template <typename C, typename T>
  PropertyBase* propertyAccess(Property<T> C::* ptr, void* instance)
  {
    C* c = reinterpret_cast<C*>(instance);
    return &((*c).*ptr);
  }

  template <typename C, typename T>
  SignalBase* signalFromInstanceProperty(Property<T> C::* ptr, void* instance)
  {
    C* c = reinterpret_cast<C*>(instance);
    Property<T>& p = ((*c).*ptr);
    return &p;
  }

  template <typename C, typename T>
  unsigned int ObjectTypeBuilderBase::advertiseEvent(const std::string& eventName, Signal<T> C::* signalAccessor, int id)
  {
    // FIXME validate type
    SignalMemberGetter fun = boost::bind(&signalAccess<C, T>, signalAccessor, _1);
    return xAdvertiseEvent(eventName + "::" + detail::FunctionSignature<T>::signature(), fun, id);
  }

  template <typename C, typename T>
  unsigned int ObjectTypeBuilderBase::advertiseProperty(const std::string& name, Property<T> C::* accessor)
  {
    // FIXME validate type
    SignalMemberGetter fun = boost::bind(&signalFromInstanceProperty<C, T>, accessor, _1);
    // advertise the event
    unsigned int id = xAdvertiseEvent(name + "::" + detail::FunctionSignature<void(const T&)>::signature(), fun);

    PropertyMemberGetter pg = boost::bind(&propertyAccess<C, T>, accessor, _1);
    return xAdvertiseProperty(name, typeOf<T>()->signature(), pg, id);
  }

  template <typename T> unsigned int ObjectTypeBuilderBase::advertiseEvent(const std::string& name, SignalMemberGetter getter, int id)
  {
    return xAdvertiseEvent(name + "::" + detail::FunctionSignature<T>::signature(), getter, id);
  }

  namespace detail
  {
    template<typename E>
    struct AdvertiseDispatch
    {
      unsigned int operator()(const std::string& name, ObjectTypeBuilderBase* b,
                              E e)
      {
        return b->advertiseMethod(name, e);
      }
    };
    template<typename T, typename C>
    struct AdvertiseDispatch<Signal<T> C::*>
    {
      unsigned int operator()(const std::string& name, ObjectTypeBuilderBase* b,
                              Signal<T> C::* e)
      {
        return b->advertiseEvent(name, e);
      }
    };
    template<typename T, typename C>
    struct AdvertiseDispatch<Property<T> C::*>
    {
      unsigned int operator()(const std::string& name, ObjectTypeBuilderBase* b,
        Property<T> C::* e)
      {
        return b->advertiseProperty(name, e);
      }
    };
  }

  template<typename E>
  unsigned int ObjectTypeBuilderBase::advertise(const std::string& name, E e)
  {
    return detail::AdvertiseDispatch<E>()(name, this, e);
  }
}


#endif  // _QITYPE_DETAILS_OBJECTTYPEBUILDER_HXX_

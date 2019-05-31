#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_OBJECTTYPEBUILDER_HXX_
#define _QITYPE_DETAIL_OBJECTTYPEBUILDER_HXX_

#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/mpl/front.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/type/metamethod.hpp>
#include <qi/actor.hpp>

namespace qi {

  namespace detail {
    template <typename T>
    qi::Strand* callWithVoid(qi::Strand* (T::*member)() const, void* instance)
    {
      return (static_cast<T*>(instance)->*member)();
    }

    template <typename T>
    typename boost::enable_if<boost::is_base_of<Actor, T>, AnyFunction>::type
        getStrandAccessor()
    {
      return AnyFunction::from(boost::function<qi::Strand*(void*)>(
          boost::bind(&callWithVoid<T>, &T::strand, _1)));
    }
    template <typename T>
    typename boost::disable_if<boost::is_base_of<Actor, T>, AnyFunction>::type
        getStrandAccessor()
    {
      return AnyFunction();
    }
  }

  template<typename T> void ObjectTypeBuilderBase::buildFor(bool autoRegister)
  {
    // We are erasing T here: we must pass everything the builder need to know about t:
    // - typeid
    // - cloner/deleter
    // - serializer, ...
    // => wee need all TypeInterface* methods, but we do not want another TypeInterface*
    // to anwser to typeOf<T>
    xBuildFor(new DefaultTypeImpl<T>(), autoRegister, detail::getStrandAccessor<T>());

    if (std::is_base_of<Actor, T>::value)
      setThreadingModel(ObjectThreadingModel_SingleThread);
    else
      setThreadingModel(ObjectThreadingModel_MultiThread);
  }

  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilderBase::advertiseMethod(const std::string& name,
                                                      FUNCTION_TYPE function,
                                                      MetaCallType threadingModel,
                                                      int id)
  {
    MetaMethodBuilder builder;
    AnyFunction f = AnyFunction::from(function);
    builder.setName(name);
    builder.setSignature(f);

    // throw on error
    return xAdvertiseMethod(builder, f, threadingModel, id);
  }

  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilderBase::advertiseMethod(MetaMethodBuilder& builder,
                                                      FUNCTION_TYPE function,
                                                      MetaCallType threadingModel,
                                                      int id)
  {
    AnyFunction f = AnyFunction::from(function);
    builder.setSignature(f);

    // throw on error
    return xAdvertiseMethod(builder, f, threadingModel, id);
  }

  template<typename U>
  void ObjectTypeBuilderBase::inherits(std::ptrdiff_t offset)
  {
    return inherits(typeOf<
      typename boost::remove_reference<U>::type>(), offset);
  }

  template<typename T>
  template<typename U>
  void ObjectTypeBuilder<T>::inherits()
  {
    qiLogCategory("qitype.objectbuilder");
    // Compute the offset between T and U
    T* ptr = reinterpret_cast<T*>(0x10000);
    U* pptr = ptr;
    std::ptrdiff_t offset = reinterpret_cast<intptr_t>(pptr) - reinterpret_cast<intptr_t>(ptr);
    qiLogDebug() << "Offset check T(" << qi::typeIdRuntime(ptr).name() << ")= " << ptr
                 << ", U(" << qi::typeIdRuntime(pptr).name() << ")= " << pptr << ", T-U= " << offset;
    return ObjectTypeBuilderBase::inherits<U>(offset);
  }

  namespace detail
  {
    template<typename F, typename T> void checkRegisterParent(
      ObjectTypeBuilder<T>& , boost::false_type) {}
    template<typename F, typename T> void checkRegisterParent(
      ObjectTypeBuilder<T>& builder, boost::true_type)
    {
      using ArgsType = typename boost::function_types::parameter_types<F>::type;
      using DecoratedClassType = typename boost::mpl::front<ArgsType>::type;
      using ClassType = typename boost::remove_reference<DecoratedClassType>::type;
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

    // throw on error
    return ObjectTypeBuilderBase::advertiseMethod(name, function, threadingModel, id);
  }

  template <typename T>
  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilder<T>::advertiseMethod(MetaMethodBuilder& name, FUNCTION_TYPE function, MetaCallType threadingModel, int id)
  {
    // Intercept advertise to auto-register parent type if this is a parent method
    // Note: if FUNCTION_TYPE is a grandparent method, we will incorrectly add it
    // as a child
    detail::checkRegisterParent<FUNCTION_TYPE>(
      *this,
      typename boost::function_types::is_member_function_pointer<FUNCTION_TYPE >::type());

    // throw on error
    return ObjectTypeBuilderBase::advertiseMethod(name, function, threadingModel, id);
  }

  template <typename T>
  AnyObject ObjectTypeBuilder<T>::object(T* ptr, boost::function<void (GenericObject*)> onDestroy)
  {
    return ObjectTypeBuilderBase::object(static_cast<void*>(ptr), onDestroy);
  }


  template<typename T>
  void ObjectTypeBuilder<T>::registerType()
  {
    ::qi::registerType(qi::typeId<T>(), type());
  }

  template<typename A>
  typename boost::enable_if<typename detail::Accessor<A>::is_accessor, SignalBase*>::type
  signalAccess(A acc, void* instance)
  {
    using class_type = typename detail::Accessor<A>::class_type;
    return &detail::Accessor<A>::access((class_type*)instance, acc);
  }

  template<typename A>
  typename boost::enable_if<typename detail::Accessor<A>::is_accessor, PropertyBase*>::type
  propertyAccess(A acc, void* instance)
  {
    using class_type = typename detail::Accessor<A>::class_type;
    return &detail::Accessor<A>::access((class_type*)instance, acc);
  }

  template<typename A>
  unsigned int
  ObjectTypeBuilderBase::advertiseSignal(const std::string& eventName, A accessor, int id, bool isSignalProperty)
  {
    SignalMemberGetter fun = boost::bind(&signalAccess<A>, accessor, _1);
    using FunctionType = typename detail::Accessor<A>::value_type::FunctionType;
    return xAdvertiseSignal(eventName,
      detail::FunctionSignature<FunctionType>::signature(), fun, id, isSignalProperty);
  }

  template <typename A>
  unsigned int ObjectTypeBuilderBase::advertiseProperty(const std::string& name, A accessor)
  {
    unsigned int id = advertiseSignal(name, accessor, -1, true);
    PropertyMemberGetter pg = boost::bind(&propertyAccess<A>, accessor, _1);
    using PropertyType = typename detail::Accessor<A>::value_type::PropertyType;
    return xAdvertiseProperty(name, typeOf<PropertyType>()->signature(), pg, id);
  }

  template <typename T> unsigned int ObjectTypeBuilderBase::advertiseSignal(const std::string& name, SignalMemberGetter getter, int id, bool isSignalProperty)
  {
    return xAdvertiseSignal(name, detail::FunctionSignature<T>::signature(), getter, id, isSignalProperty);
  }

  template<typename T>
  inline unsigned int ObjectTypeBuilderBase::advertiseProperty(const std::string& eventName, PropertyMemberGetter getter)
  {
    return xAdvertiseProperty(eventName, typeOf<T>()->signature(), getter);
  }


  namespace detail
  {
    static const char* interfaceMarker = "_interface_";
    static const auto interfaceMarkerLength = strlen(interfaceMarker);

    // Trait that detect inheritance from PropertyBase SignalBase or none of the above.

    template<typename T, bool b> struct SigPropInheritsSignal
    {};
    template<typename T, bool b> struct SigPropInheritsProperty
    {};

    template<typename T> struct SigProp : public
    SigPropInheritsProperty<T, boost::is_base_of<PropertyBase, T>::value> {};

    template<typename T> struct SigPropInheritsProperty<T, true>
    {
      static const unsigned value = 2;
    };
    template<typename T> struct SigPropInheritsProperty<T, false>
    : public SigPropInheritsSignal<T, boost::is_base_of<SignalBase, T>::value> {};

    template<typename T> struct SigPropInheritsSignal<T, true>
    {
       static const unsigned value = 1;
    };
    template<typename T> struct SigPropInheritsSignal<T, false>
    {
       static const unsigned value = 0;
    };

    template<unsigned> struct Dummy {};
    template<typename A> unsigned int advertise(ObjectTypeBuilderBase* builder, const std::string& name, A accessor, Dummy<0>)
    {
      return builder->advertiseMethod(name, accessor);
    }
    template<typename A> unsigned int advertise(ObjectTypeBuilderBase* builder, const std::string& name, A accessor, Dummy<1>)
    {
      std::string n = name;
      if (n.size() > interfaceMarkerLength && n.substr(0, interfaceMarkerLength) == interfaceMarker)
        n = name.substr(interfaceMarkerLength);
      return builder->advertiseSignal(n, accessor);
    }
    template<typename A> unsigned int advertise(ObjectTypeBuilderBase* builder, const std::string& name, A accessor, Dummy<2>)
    {
      std::string n = name;
      if (n.size() > interfaceMarkerLength && n.substr(0, interfaceMarkerLength) == interfaceMarker)
        n = name.substr(interfaceMarkerLength);
      return builder->advertiseProperty(n, accessor);
    }
    template<typename A> unsigned int advertiseBounce(ObjectTypeBuilderBase* builder, const std::string& name, A accessor, boost::true_type)
    {
      return advertise(builder, name, accessor,
        Dummy<detail::SigProp<typename detail::Accessor<A>::value_type>::value>());
    }
    template<typename A> unsigned int advertiseBounce(ObjectTypeBuilderBase* builder, const std::string& name, A accessor, boost::false_type)
    {
      return builder->advertiseMethod(name, accessor);
    }
  }
  template<typename T>
  unsigned int
  ObjectTypeBuilderBase::advertiseId(const std::string& name, T element)
  {
    return detail::advertiseBounce(this, name, element, typename detail::Accessor<T>::is_accessor());
  }
  template<typename T>
  ObjectTypeBuilderBase&
  ObjectTypeBuilderBase::advertise(const std::string& name, T element)
  {
    detail::advertiseBounce(this, name, element, typename detail::Accessor<T>::is_accessor());
    return *this;
  }

}


#endif  // _QITYPE_DETAIL_OBJECTTYPEBUILDER_HXX_

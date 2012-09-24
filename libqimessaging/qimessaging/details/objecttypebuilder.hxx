#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_OBJECTTYPEBUILDER_HXX_
#define _QIMESSAGING_DETAILS_OBJECTTYPEBUILDER_HXX_


namespace qi {

  namespace detail {

    template<typename T>
    Manageable* manageable(void* instance)
    {
      return reinterpret_cast<T*>(instance);
    }

    template<typename T, typename U> boost::function<Manageable*(void*)>
    getAsManageable(U)
    {
      return boost::function<Manageable*(void*)>();
    }

    template<typename T> boost::function<Manageable*(void*)>
    getAsManageable(boost::true_type)
    {
      return manageable<T>;
    }
  }

  template<typename T> void ObjectTypeBuilderBase::buildFor()
  {
    // We are erasing T here: we must pass everything the builder need to know about t:
    // - manageable accessor
    // - typeid
    // - cloner/deleter
    // - serializer, ...
    // => wee need all Type* methods, but we do not want another Type*
    // to anwser to typeOf<T>
    xBuildFor(new DefaultTypeImpl<T>(),
      detail::getAsManageable<T>(typename boost::is_base_of<Manageable, T>::type()));
  }

  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilderBase::advertiseMethod(const std::string& name, FUNCTION_TYPE function)
  {
    // FIXME validate type
    return xAdvertiseMethod(detail::FunctionSignature<FUNCTION_TYPE>::sigreturn(),
      name + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature(),
      makeGenericMethod(function));
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
    T* ptr = (T*)(void*)0x10000;
    U* pptr = ptr;
    int offset = (long)(void*)pptr - (long)(void*) ptr;
    qiLogDebug("qi.meta") << "Offset check " << pptr <<" " << ptr << " " << offset;
    qiLogDebug("qi.meta") << typeid(ptr).name() << " " << typeid(pptr).name();
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
  unsigned int ObjectTypeBuilder<T>::advertiseMethod(const std::string& name, FUNCTION_TYPE function)
  {
    // Intercept advertise to auto-register parent type if this is a parent method
    // Note: if FUNCTION_TYPE is a grandparent method, we will incorrectly add it
    // as a child
    detail::checkRegisterParent<FUNCTION_TYPE>(
      *this,
      typename boost::function_types::is_member_function_pointer<FUNCTION_TYPE >::type());
    return ObjectTypeBuilderBase::advertiseMethod(name, function);
  }

  template<typename T>
  void ObjectTypeBuilder<T>::registerType()
  {
    ::qi::registerType(typeid(T), type());
  }

  template <typename C, typename T>
  SignalBase signalAccess(Signal<T> C::* ptr, void* instance)
  {
    C* c = reinterpret_cast<C*>(instance);
    return (*c).*ptr;
  }

  template <typename C, typename T>
  unsigned int ObjectTypeBuilderBase::advertiseEvent(const std::string& eventName, Signal<T> C::* signalAccessor)
  {
    // FIXME validate type
    SignalMemberGetter fun = boost::bind(&signalAccess<C, T>, signalAccessor, _1);
    return xAdvertiseEvent(eventName + "::" + detail::FunctionSignature<T>::signature(), fun);
  }

  template <typename T> unsigned int ObjectTypeBuilderBase::advertiseEvent(const std::string& name, SignalMemberGetter getter)
  {
    return xAdvertiseEvent(name + "::" + detail::FunctionSignature<T>::signature(), getter);
  }


}


#endif  // _QIMESSAGING_DETAILS_OBJECTTYPEBUILDER_HXX_

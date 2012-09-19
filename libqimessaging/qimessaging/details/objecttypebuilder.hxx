/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/
#ifndef _QIMESSAGING_OBJECTTYPEBUILDER_HXX_
#define _QIMESSAGING_OBJECTTYPEBUILDER_HXX_


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
    xBuildFor(typeOf<T*>(),
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


#endif

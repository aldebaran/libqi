/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_GENERICOBJECTBUILDER_HXX_
#define _QIMESSAGING_GENERICOBJECTBUILDER_HXX_

namespace qi {

template <typename FUNCTION_TYPE>
  unsigned int GenericObjectBuilder::advertiseMethod(const std::string& name, FUNCTION_TYPE function)
  {
    // FIXME validate type
    return xAdvertiseMethod(detail::FunctionSignature<FUNCTION_TYPE>::sigreturn(),
      name + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature(),
      makeCallable(function));
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method)
  {
    return xAdvertiseMethod(detail::FunctionSignature<METHOD_TYPE >::sigreturn(),
      name + "::" + detail::FunctionSignature<METHOD_TYPE >::signature(),
      makeCallable(makeGenericFunction(object, method)));
  }

  template <typename T> unsigned int GenericObjectBuilder::advertiseEvent(const std::string& name)
  {
    return xAdvertiseEvent(name + "::" + detail::FunctionSignature<T>::signature());
  }

}
#endif

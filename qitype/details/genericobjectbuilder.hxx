#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICOBJECTBUILDER_HXX_
#define _QITYPE_DETAILS_GENERICOBJECTBUILDER_HXX_

namespace qi {

template <typename FUNCTION_TYPE>
  unsigned int GenericObjectBuilder::advertiseMethod(const std::string& name, FUNCTION_TYPE function, MetaCallType threadingModel)
  {
    // FIXME validate type
    return xAdvertiseMethod(detail::FunctionSignature<FUNCTION_TYPE>::sigreturn(),
      name + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature(),
      makeGenericFunction(function), threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method, MetaCallType threadingModel)
  {
    return xAdvertiseMethod(detail::FunctionSignature<METHOD_TYPE >::sigreturn(),
      name + "::" + detail::FunctionSignature<METHOD_TYPE >::signature(),
      makeGenericFunction(object, method), threadingModel);
  }

  template <typename T> unsigned int GenericObjectBuilder::advertiseEvent(const std::string& name)
  {
    return xAdvertiseEvent(name + "::" + detail::FunctionSignature<T>::signature());
  }

}
#endif  // _QITYPE_DETAILS_GENERICOBJECTBUILDER_HXX_

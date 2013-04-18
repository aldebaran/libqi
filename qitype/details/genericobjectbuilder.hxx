#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICOBJECTBUILDER_HXX_
#define _QITYPE_DETAILS_GENERICOBJECTBUILDER_HXX_

#include <qitype/genericobjectbuilder.hpp>
#include <qitype/metamethod.hpp>

namespace qi {

  template <typename FUNCTION_TYPE>
  unsigned int GenericObjectBuilder::advertiseMethod(const std::string& name,
                                                     FUNCTION_TYPE function,
                                                     const std::string& desc,
                                                     MetaCallType threadingModel)
  {
    MetaMethodBuilder builder(name, desc);
    GenericFunction f = makeGenericFunction(function).dropFirstArgument();
    builder.setSignatures(f);
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(const std::string& name,
                                                            OBJECT_TYPE object,
                                                            METHOD_TYPE method,
                                                            const std::string& desc,
                                                            MetaCallType threadingModel)
  {
    MetaMethodBuilder builder(name, desc);
    GenericFunction f = makeGenericFunction(method, object).dropFirstArgument();
    builder.setSignatures(f);
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename FUNCTION_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(MetaMethodBuilder& builder,
                                                            FUNCTION_TYPE function,
                                                            MetaCallType threadingModel)
  {
    GenericFunction f = makeGenericFunction(function).dropFirstArgument();
    builder.setSignatures(f);
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(MetaMethodBuilder& builder,
                                                            OBJECT_TYPE object,
                                                            METHOD_TYPE method,
                                                            MetaCallType threadingModel)
  {
    GenericFunction f = makeGenericFunction(method, object).dropFirstArgument();
    builder.setSignatures(f);
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename T> unsigned int GenericObjectBuilder::advertiseEvent(const std::string& name)
  {
    return xAdvertiseEvent(name + "::" + detail::FunctionSignature<T>::signature());
  }

  template <typename T> unsigned int GenericObjectBuilder::advertiseProperty(const std::string& name)
  {
    // we must end up with name event, get_name and set_name methods
    unsigned int isig = advertiseEvent<void(const T&)>(name);
    isig = xAdvertiseProperty(name, typeOf<T>()->signature(), isig);
    return isig;
  }

}
#endif  // _QITYPE_DETAILS_GENERICOBJECTBUILDER_HXX_

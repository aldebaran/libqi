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
    // FIXME validate type
    MetaMethodBuilder builder(name, desc);
    builder.setSigreturn(detail::FunctionSignature<FUNCTION_TYPE>::sigreturn());
    builder.setSignature(name + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature());
    return xAdvertiseMethod(builder, makeGenericFunction(function), threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(const std::string& name,
                                                            OBJECT_TYPE object,
                                                            METHOD_TYPE method,
                                                            const std::string& desc,
                                                            MetaCallType threadingModel)
  {
    MetaMethodBuilder builder(name, desc);
    builder.setSigreturn(detail::FunctionSignature<METHOD_TYPE>::sigreturn());
    builder.setSignature(name + "::" + detail::FunctionSignature<METHOD_TYPE>::signature());
    return xAdvertiseMethod(builder, makeGenericFunction(object, method), threadingModel);
  }

  template <typename FUNCTION_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(MetaMethodBuilder& builder,
                                                            FUNCTION_TYPE function,
                                                            MetaCallType threadingModel)
  {
    builder.setSigreturn(detail::FunctionSignature<FUNCTION_TYPE>::sigreturn());
    builder.setSignature(builder.name() + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature());
    return xAdvertiseMethod(builder, makeGenericFunction(function), threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(MetaMethodBuilder& builder,
                                                            OBJECT_TYPE object,
                                                            METHOD_TYPE method,
                                                            MetaCallType threadingModel)
  {
    builder.setSigreturn(detail::FunctionSignature<METHOD_TYPE>::sigreturn());
    builder.setSignature(builder.name() + "::" + detail::FunctionSignature<METHOD_TYPE>::signature());
    return xAdvertiseMethod(builder, makeGenericFunction(object, method), threadingModel);
  }

  template <typename T> unsigned int GenericObjectBuilder::advertiseEvent(const std::string& name)
  {
    return xAdvertiseEvent(name + "::" + detail::FunctionSignature<T>::signature());
  }

}
#endif  // _QITYPE_DETAILS_GENERICOBJECTBUILDER_HXX_

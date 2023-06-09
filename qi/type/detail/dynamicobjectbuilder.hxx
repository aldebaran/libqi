#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_GENERICOBJECTBUILDER_HXX_
#define _QITYPE_DETAIL_GENERICOBJECTBUILDER_HXX_

#include <qi/type/dynamicobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/metamethod.hpp>

namespace qi {

  template <typename FUNCTION_TYPE>
  unsigned int DynamicObjectBuilder::advertiseMethod(const std::string& name,
                                                     FUNCTION_TYPE function,
                                                     const std::string& desc,
                                                     MetaCallType threadingModel)
  {
    MetaMethodBuilder builder;
    AnyFunction f = AnyFunction::from(std::move(function)).dropFirstArgument();
    builder.setName(name);
    builder.setSignature(f);
    builder.setDescription(desc);
    // throw on error
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int DynamicObjectBuilder::advertiseMethod(const std::string& name,
                                                            OBJECT_TYPE object,
                                                            METHOD_TYPE method,
                                                            const std::string& desc,
                                                            MetaCallType threadingModel)
  {
    MetaMethodBuilder builder;
    AnyFunction f = AnyFunction::from(std::move(method), std::move(object)).dropFirstArgument();
    builder.setName(name);
    builder.setSignature(f);
    builder.setDescription(desc);
    // throw on error
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename FUNCTION_TYPE>
  inline unsigned int DynamicObjectBuilder::advertiseMethod(MetaMethodBuilder& builder,
                                                            FUNCTION_TYPE function,
                                                            MetaCallType threadingModel)
  {
    AnyFunction f = AnyFunction::from(std::move(function)).dropFirstArgument();
    builder.setSignature(f);
    // throw on error
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int DynamicObjectBuilder::advertiseMethod(MetaMethodBuilder& builder,
                                                            OBJECT_TYPE object,
                                                            METHOD_TYPE method,
                                                            MetaCallType threadingModel)
  {
    AnyFunction f = AnyFunction::from(std::move(method), std::move(object)).dropFirstArgument();
    builder.setSignature(f);
    // throw on error
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename... Args>
  unsigned int DynamicObjectBuilder::advertiseSignal(const std::string& s)
  {
    return advertiseSignalF<void(Args...)>(s);
  }

  template <typename T> unsigned int DynamicObjectBuilder::advertiseSignalF(const std::string& name)
  {
    return xAdvertiseSignal(name, detail::functionArgumentsSignature<T>());
  }

  template <typename T> unsigned int DynamicObjectBuilder::advertiseProperty(const std::string& name)
  {
    // we must end up with name event, get_name and set_name methods
    unsigned int isig = advertiseSignal<const T&>(name);
    isig = xAdvertiseProperty(name, typeOf<T>()->signature(), isig);
    return isig;
  }

  template<typename T> qi::AnyObject DynamicObjectBuilder::object(boost::shared_ptr<T> other)
  {
    return object([other](qi::GenericObject*) mutable { other.reset(); });
  }

}
#endif  // _QITYPE_DETAIL_GENERICOBJECTBUILDER_HXX_

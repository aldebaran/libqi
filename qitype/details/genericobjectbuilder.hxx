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
    MetaMethodBuilder builder;
    GenericFunction f = makeGenericFunction(function).dropFirstArgument();
    builder.setName(name);
    builder.setSignature(f);
    builder.setDescription(desc);
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(const std::string& name,
                                                            OBJECT_TYPE object,
                                                            METHOD_TYPE method,
                                                            const std::string& desc,
                                                            MetaCallType threadingModel)
  {
    MetaMethodBuilder builder;
    GenericFunction f = makeGenericFunction(method, object).dropFirstArgument();
    builder.setName(name);
    builder.setSignature(f);
    builder.setDescription(desc);
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename FUNCTION_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(MetaMethodBuilder& builder,
                                                            FUNCTION_TYPE function,
                                                            MetaCallType threadingModel)
  {
    GenericFunction f = makeGenericFunction(function).dropFirstArgument();
    builder.setSignature(f);
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  template <typename OBJECT_TYPE, typename METHOD_TYPE>
  inline unsigned int GenericObjectBuilder::advertiseMethod(MetaMethodBuilder& builder,
                                                            OBJECT_TYPE object,
                                                            METHOD_TYPE method,
                                                            MetaCallType threadingModel)
  {
    GenericFunction f = makeGenericFunction(method, object).dropFirstArgument();
    builder.setSignature(f);
    return xAdvertiseMethod(builder, f, threadingModel);
  }

  #define gen(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma) \
  QI_GEN_MAYBE_TEMPLATE_OPEN(comma) ATYPEDECL QI_GEN_MAYBE_TEMPLATE_CLOSE(comma) \
  inline int GenericObjectBuilder::advertiseSignal(const std::string& s) \
  { \
    return advertiseSignalF<void(ATYPES)>(s); \
  }

  QI_GEN_RANGE(gen, 8)
  #undef gen

  template <typename T> int GenericObjectBuilder::advertiseSignalF(const std::string& name)
  {
    return xAdvertiseSignal(name, detail::FunctionSignature<T>::signature());
  }

  template <typename T> unsigned int GenericObjectBuilder::advertiseProperty(const std::string& name)
  {
    // we must end up with name event, get_name and set_name methods
    unsigned int isig = advertiseSignal<const T&>(name);
    isig = xAdvertiseProperty(name, typeOf<T>()->signature(), isig);
    return isig;
  }

  namespace detail
  {
    // create an object with a single method name fname bouncing to func
    template<typename T> ObjectPtr makeObject(const std::string& fname, T func)
    {
      GenericObjectBuilder gob;
      gob.advertiseMethod(fname, func);
      return gob.object();
    }
  }
}
#endif  // _QITYPE_DETAILS_GENERICOBJECTBUILDER_HXX_

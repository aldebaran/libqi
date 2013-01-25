#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_GENERICOBJECTBUILDER_HPP_
#define _QITYPE_GENERICOBJECTBUILDER_HPP_

#include <qitype/api.hpp>
#include <qitype/genericobject.hpp>

namespace qi {

  class DynamicObject;
  class GenericObjectBuilderPrivate;
  class QITYPE_API GenericObjectBuilder
  {
  public:
    GenericObjectBuilder();
    GenericObjectBuilder(DynamicObject *dynobject, bool deleteOnDestroy = true);

    ~GenericObjectBuilder();

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    inline unsigned int advertiseMethod(const std::string& name,
                                        OBJECT_TYPE object,
                                        METHOD_TYPE method,
                                        const std::string& desc = "",
                                        MetaCallType threadingModel = MetaCallType_Auto);

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name,
                                        FUNCTION_TYPE function,
                                        const std::string& desc = "",
                                        MetaCallType threadingModel = MetaCallType_Auto);

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    inline unsigned int advertiseMethod(MetaMethodBuilder& builder,
                                        OBJECT_TYPE object,
                                        METHOD_TYPE method,
                                        MetaCallType threadingModel = MetaCallType_Auto);

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(MetaMethodBuilder& builder,
                                        FUNCTION_TYPE function,
                                        MetaCallType threadingModel = MetaCallType_Auto);

    template<typename FUNCTION_TYPE>
    inline unsigned int advertiseEvent(const std::string& eventName);

    void setThreadingModel(ObjectThreadingModel model);

    int xAdvertiseMethod(const std::string& sigret, const std::string& signature,
                         GenericFunction func, const std::string& desc = "",
                         MetaCallType threadingModel = MetaCallType_Auto);

    int xAdvertiseMethod(MetaMethodBuilder& builder, GenericFunction func,
                         MetaCallType threadingModel = MetaCallType_Auto);

    int xAdvertiseEvent(const std::string& signature);

    void setDescription(const std::string& desc);

    qi::ObjectPtr object();
  public:
    GenericObjectBuilderPrivate *_p;
    QI_DISALLOW_COPY_AND_ASSIGN(GenericObjectBuilder);
  };
}

#include <qitype/details/genericobjectbuilder.hxx>

#endif  // _QITYPE_GENERICOBJECTBUILDER_HPP_

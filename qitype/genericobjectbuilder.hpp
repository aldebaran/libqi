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
  class DynamicObjectBuilderPrivate;
  class QITYPE_API DynamicObjectBuilder
  {
  public:
    DynamicObjectBuilder();
    DynamicObjectBuilder(DynamicObject *dynobject, bool deleteOnDestroy = true);

    ~DynamicObjectBuilder();

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

    int advertiseSignal(const std::string& name);
    template<typename P0> int advertiseSignal(const std::string& name);
    template<typename P0, typename P1> int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2> int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3> int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3,typename P4> int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3,typename P4,typename P5> int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3,typename P4,typename P5,typename P6> int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3,typename P4,typename P5,typename P6, typename P7> int advertiseSignal(const std::string& name);

    /// Advertise a signal using a function signature
    template<typename T>
    int advertiseSignalF(const std::string& name);

    int advertiseSignal(const std::string &name, qi::SignalBase *signal);

    template<typename T>
    unsigned int advertiseProperty(const std::string& name);
    /// Ownership is transferred to the object
    int advertiseProperty(const std::string &name, qi::PropertyBase *sig);

    void setThreadingModel(ObjectThreadingModel model);

    int xAdvertiseMethod(const Signature &sigret,
                         const std::string &name,
                         const Signature &signature,
                         AnyFunction func, const std::string& desc = "",
                         MetaCallType threadingModel = MetaCallType_Auto);

    int xAdvertiseMethod(MetaMethodBuilder& builder, AnyFunction func,
                         MetaCallType threadingModel = MetaCallType_Auto);

    int xAdvertiseSignal(const std::string &name, const Signature &signature);
    int xAdvertiseProperty(const std::string& name, const Signature& sig, int id=-1);
    void setDescription(const std::string& desc);
    qi::AnyObject object(boost::function<void (GenericObject*)> onDelete = boost::function<void (GenericObject*)>());

    void markProperty(unsigned int ev, unsigned int getter, unsigned int setter);
  public:
    DynamicObjectBuilderPrivate *_p;
    QI_DISALLOW_COPY_AND_ASSIGN(DynamicObjectBuilder);
  };
}

#include <qitype/details/genericobjectbuilder.hxx>

#endif  // _QITYPE_GENERICOBJECTBUILDER_HPP_

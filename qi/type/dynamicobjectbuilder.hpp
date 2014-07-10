#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_DYNAMICOBJECTBUILDER_HPP_
#define _QI_TYPE_DYNAMICOBJECTBUILDER_HPP_

#include <boost/noncopyable.hpp>
#include <qi/anyobject.hpp>
#include <qi/property.hpp>

namespace qi {

  class DynamicObject;
  class DynamicObjectBuilderPrivate;
  class QI_API DynamicObjectBuilder : private boost::noncopyable
  {
  public:
    DynamicObjectBuilder();
    DynamicObjectBuilder(DynamicObject *dynobject, bool deleteOnDestroy = true);

    ~DynamicObjectBuilder();

    // throw on error
    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    inline unsigned int advertiseMethod(const std::string& name,
                                        OBJECT_TYPE object,
                                        METHOD_TYPE method,
                                        const std::string& desc = "",
                                        MetaCallType threadingModel = MetaCallType_Auto);

    // throw on error
    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name,
                                        FUNCTION_TYPE function,
                                        const std::string& desc = "",
                                        MetaCallType threadingModel = MetaCallType_Auto);

    // throw on error
    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    inline unsigned int advertiseMethod(MetaMethodBuilder& builder,
                                        OBJECT_TYPE object,
                                        METHOD_TYPE method,
                                        MetaCallType threadingModel = MetaCallType_Auto);

    // throw on error
    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(MetaMethodBuilder& builder,
                                        FUNCTION_TYPE function,
                                        MetaCallType threadingModel = MetaCallType_Auto);

    /** create a T, wrap in a AnyObject
     *  All template parameters are given to the T constructor except the first one
     */
    #define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                 \
    template<typename T comma ATYPEDECL>                                      \
    inline unsigned int advertiseFactory(const std::string& name)             \
    {                                                                         \
      return advertiseMethod(name, &constructObject<T comma ATYPES>);         \
    }
    QI_GEN(genCall)
    #undef genCall

    unsigned int advertiseSignal(const std::string& name);
    template<typename P0> unsigned int advertiseSignal(const std::string& name);
    template<typename P0, typename P1> unsigned int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2> unsigned int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3> unsigned int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3,typename P4> unsigned int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3,typename P4,typename P5> unsigned int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3,typename P4,typename P5,typename P6> unsigned int advertiseSignal(const std::string& name);
    template<typename P0, typename P1, typename P2, typename P3,typename P4,typename P5,typename P6, typename P7> unsigned int advertiseSignal(const std::string& name);

    /// Advertise a signal using a function signature
    template<typename T>
    unsigned int advertiseSignalF(const std::string& name);

    unsigned int advertiseSignal(const std::string &name, qi::SignalBase *signal);

    template<typename T>
    unsigned int advertiseProperty(const std::string& name);
    /// Ownership is transferred to the object
    unsigned int advertiseProperty(const std::string &name, qi::PropertyBase *sig);

    void setThreadingModel(ObjectThreadingModel model);

    unsigned int xAdvertiseMethod(const Signature &sigret,
                                  const std::string &name,
                                  const Signature &signature,
                                  AnyFunction func, const std::string& desc = "",
                                  MetaCallType threadingModel = MetaCallType_Auto);

    unsigned int xAdvertiseMethod(MetaMethodBuilder& builder, AnyFunction func,
                                  MetaCallType threadingModel = MetaCallType_Auto);

    unsigned int xAdvertiseSignal(const std::string &name, const Signature &signature);
    unsigned int xAdvertiseProperty(const std::string& name, const Signature& sig, int id=-1);
    void setDescription(const std::string& desc);
    qi::AnyObject object(boost::function<void (GenericObject*)> onDelete = boost::function<void (GenericObject*)>());
    /// Return an AnyObject that shares life type with \p other.
    template<typename T> qi::AnyObject object(boost::shared_ptr<T> other);
    void markProperty(unsigned int ev, unsigned int getter, unsigned int setter);
  private:
    DynamicObject* bareObject();
    void setManageable(DynamicObject* obj, Manageable* m);
    DynamicObjectBuilderPrivate *_p;
  };
}

#include <qi/type/detail/dynamicobjectbuilder.hxx>

#endif  // _QITYPE_DYNAMICOBJECTBUILDER_HPP_

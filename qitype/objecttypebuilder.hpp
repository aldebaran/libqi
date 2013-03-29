#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_OBJECTTYPEBUILDER_HPP_
#define _QITYPE_OBJECTTYPEBUILDER_HPP_

#include <qitype/api.hpp>
#include <string>

#include <boost/function.hpp>
#include <qitype/signature.hpp>
#include <sstream>
#include <qitype/type.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/property.hpp>


namespace qi {

  class MetaObject;

  class SignalBase;
  class ObjectType;
  class Type;
  template<typename T> class Signal;
  class ObjectTypeBuilderPrivate;

  class QITYPE_API ObjectTypeBuilderBase
  {
  public:
    ObjectTypeBuilderBase();
    ~ObjectTypeBuilderBase();

    typedef boost::function<SignalBase* (void*)> SignalMemberGetter;
    typedef boost::function<PropertyBase* (void*)> PropertyMemberGetter;

    // input: template-based

    /// Declare the class type for which this StaticBuilder is.
    template<typename T> void  buildFor();
    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function, MetaCallType threadingModel = MetaCallType_Auto, int id = -1);
    template <typename C, typename T>
    inline unsigned int advertiseEvent(const std::string& eventName, Signal<T> C::* signalAccessor, int id = -1);
    template <typename T>
    inline unsigned int advertiseEvent(const std::string& name, SignalMemberGetter getter, int id = -1);

    template <typename C, typename T>
    inline unsigned int advertiseProperty(const std::string& eventName, Property<T> C::* propertyAccessor);

    template<typename P> void inherits(int offset);

    // Advertise anything, dispatch on {method, event, property} based on T.
    template<typename T>
    unsigned int advertise(const std::string& name, T element);
    // input: type-erased

    int xAdvertiseMethod(MetaMethodBuilder& builder, GenericFunction func, MetaCallType threadingModel = MetaCallType_Auto, int id = -1);

    int xAdvertiseEvent(const std::string& signature, SignalMemberGetter getter, int id = -1);
    int xAdvertiseProperty(const std::string& name, const std::string& sig, PropertyMemberGetter getter, int id = -1);
    void xBuildFor(Type* type);
    void inherits(Type* parentType, int offset);

    // Configuration

    void setThreadingModel(ObjectThreadingModel model);

    // output
    const MetaObject& metaObject();
    ObjectPtr object(void* ptr, boost::function<void (GenericObject*)> onDestroy = boost::function<void (GenericObject*)>());
    ObjectType* type();

    /// Register type to typeof. Called by type()
    inline virtual void registerType() {};

  private:
    ObjectTypeBuilderPrivate* _p;
  };

  template<typename T>
  class ObjectTypeBuilder : public ObjectTypeBuilderBase
  {
  public:
    ObjectTypeBuilder()
    {
      buildFor<T>();
    }

    template<typename U> void inherits();

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name,
                                        FUNCTION_TYPE function,
                                        MetaCallType threadingModel = MetaCallType_Auto,
                                        int id = -1);

    /// Register type to typeOf<T>, to avoid both TypeImpl<T> and type() being present
    inline virtual void registerType();

    inline ObjectPtr object(T* ptr, boost::function<void (GenericObject*)> onDestroy = boost::function<void (GenericObject*)>());

  };
}

#include <qitype/details/objecttypebuilder.hxx>

#endif  // _QITYPE_OBJECTTYPEBUILDER_HPP_

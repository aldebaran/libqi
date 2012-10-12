#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_OBJECTTYPEBUILDER_HPP_
#define _QIMESSAGING_OBJECTTYPEBUILDER_HPP_

#include <qimessaging/api.hpp>
#include <string>

#include <boost/function.hpp>
#include <qimessaging/signature.hpp>
#include <sstream>
#include <qimessaging/type.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/methodtype.hpp>


namespace qi {

  class MetaObject;

  class SignalBase;
  class ObjectType;
  class Type;
  template<typename T> class Signal;
  class ObjectTypeBuilderPrivate;

  class QIMESSAGING_API ObjectTypeBuilderBase
  {
  public:
    ObjectTypeBuilderBase();
    ~ObjectTypeBuilderBase();

    typedef boost::function<SignalBase* (void*)> SignalMemberGetter;

    // input: template-based

    /// Declare the class type for which this StaticBuilder is.
    template<typename T> void  buildFor();
    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function, int id = -1);
    template <typename C, typename T>
    inline unsigned int advertiseEvent(const std::string& eventName, Signal<T> C::* signalAccessor, int id = -1);
    template <typename T>
    inline unsigned int advertiseEvent(const std::string& name, SignalMemberGetter getter, int id = -1);

    template<typename P> void inherits(int offset);

    // input: type-erased

    int xAdvertiseMethod(const std::string &retsig, const std::string& signature, GenericMethod func, int id = -1);
    int xAdvertiseEvent(const std::string& signature, SignalMemberGetter getter, int id = -1);
    void xBuildFor(Type* type, boost::function<Manageable* (void*)> asManageable);
    void inherits(Type* parentType, int offset);

    // output
    const MetaObject& metaObject();
    ObjectPtr object(void* ptr);
    ObjectType* type();

    /// Register type to typeof. Called by type()
    virtual void registerType() {};
  private:
    ObjectTypeBuilderPrivate* _p;
  };

  template<typename T> class QIMESSAGING_API ObjectTypeBuilder
  : public ObjectTypeBuilderBase
  {
  public:
    ObjectTypeBuilder()
    {
      buildFor<T>();
    }

    template<typename U> void inherits();

    template <typename FUNCTION_TYPE>
    inline unsigned int advertiseMethod(const std::string& name, FUNCTION_TYPE function, int id = -1);

    /// Register type to typeOf<T>, to avoid both TypeImpl<T> and type() being present
    virtual void registerType();

  };
}

#include <qimessaging/details/objecttypebuilder.hxx>

#endif  // _QIMESSAGING_OBJECTTYPEBUILDER_HPP_

#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_PROXYPROPERTY_HPP_
#define _QI_TYPE_PROXYPROPERTY_HPP_

#include <qi/log.hpp>
#include <qi/property.hpp>
#include <qi/anyfunction.hpp>
#include "proxysignal.hpp"

namespace qi
{
  namespace details_proxyproperty
  {
    constexpr inline const char* objectExpiredError()
    {
      return "The object that the proxy property represents has expired.";
    }

    template<typename T>
    struct Getter
    {
      using result_type = Future<T>;

    // Regular:
      KA_GENERATE_FRIEND_REGULAR_OPS_0(Getter)

    // Procedure:
      result_type operator()(AnyWeakObject weakObject, const std::string& propertyName) const
      {
        auto object = weakObject.lock();
        if (!object)
          return makeFutureError<T>(objectExpiredError());
        return object.property<T>(propertyName);
      }
    };

    template<typename T>
    struct Setter
    {
      using result_type = Future<bool>;

    // Regular:
      KA_GENERATE_FRIEND_REGULAR_OPS_0(Setter)

    // Procedure:
      result_type operator()(T& /*target*/,
                             const T& v,
                             AnyWeakObject weakObject,
                             const std::string& propertyName) const
      {
        auto object = weakObject.lock();
        if (!object)
          return makeFutureError<bool>(objectExpiredError());

        // no need to fill target it's never used since we have a getter
        return object.setProperty(propertyName, v).async().andThen([](bool){
          // Prevent local subscribers from being called, they will be notified by our own callback
          // connected to the remote signaling property once we receive the update event.
          return false;
        });
      }
    };

    template<typename T>
    void setUpProxy(PropertyImpl<T>& prop, AnyWeakObject object, const std::string& propertyName)
    {
      // signal part
      details_proxysignal::setUpProxy(prop, object, propertyName);

      // property part
      prop._asyncGetter = boost::bind(Getter<T>{}, object, propertyName);
      prop._asyncSetter = boost::bind(Setter<T>{}, _1, _2, object, propertyName);
    }

    template<typename T>
    void tearDown(PropertyImpl<T>& prop)
    {
      details_proxysignal::tearDownProxy(prop);
    }
  } // namespace details_proxyproperty

  /** Property proxy using AnyObject as backend
  * @warning reading and writing the property are synchronous operations
  * and might take time if backend object is a remote proxy.
  */
  template < typename T, template< class...> class PropertyType = Property >
  class ProxyProperty: public PropertyType<T>
  {
  public:
    using SignalType = SignalF<void (const T&)>;
    using ThisProxyType = ProxyProperty;

    ProxyProperty()
    {
    }

    ProxyProperty(AnyObject object, const std::string& propertyName)
    {
      details_proxyproperty::setUpProxy(*this, object, propertyName);
    }

    ~ProxyProperty()
    {
      details_proxyproperty::tearDown(*this);
    }

    QI_API_DEPRECATED_MSG(This function might be removed in the future with no replacement)
    AnyReference bounceEvent(const AnyReferenceVector args)
    {
      return asProxySignal().bounceEvent(args);
    }

    QI_API_DEPRECATED_MSG(This function might be removed in the future with no replacement)
    void triggerOverride(const GenericFunctionParameters& params,
                         MetaCallType callType,
                         GenericObject* object,
                         const std::string& propertyName)
    {
      return asProxySignal().triggerOverride(params, callType, object, propertyName);
    }


    QI_API_DEPRECATED_MSG(This function might be removed in the future with no replacement)
    Future<void> onSubscribe(bool enable,
                             GenericObject* genericObject,
                             const std::string& propertyName,
                             SignalLink link)
    {
      return asProxySignal().onSubscribe(enable, genericObject, propertyName, link);
    }

  private:
    ProxySignal<T>& asProxySignal() const
    {
      return static_cast<ProxySignal<T>&>(*this);
    }
  };

  template<typename T, template< class...> class PropertyType>
  void makeProxyProperty(PropertyType<T>& target, AnyObject object, const std::string& signalName)
  {
    details_proxyproperty::setUpProxy(target, object, signalName);
  }

  template<typename T, template< class...> class PropertyType>
  void makeProxyProperty(ProxyProperty<T, PropertyType>& target, AnyObject object, const std::string& signalName)
  {
    details_proxyproperty::setUpProxy(target, object, signalName);
  }

} // namespace qi

#endif  // _QITYPE_PROXYPROPERTY_HPP_

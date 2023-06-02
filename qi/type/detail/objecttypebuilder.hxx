#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_OBJECTTYPEBUILDER_HXX_
#define _QITYPE_DETAIL_OBJECTTYPEBUILDER_HXX_

#include <boost/callable_traits.hpp>
#include <boost/algorithm/string.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <qi/type/metamethod.hpp>
#include <qi/type/detail/functionsignature.hxx>
#include <qi/type/detail/accessor.hxx>
#include <qi/actor.hpp>
#include <ka/macro.hpp>
#include <string_view>
#include <type_traits>

namespace qi {

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4068, pragmas)
KA_WARNING_DISABLE(, noexcept-type)

  namespace detail {
    template <typename T>
    qi::Strand* callWithVoid(qi::Strand* (T::*member)() const, void* instance)
    {
      return (static_cast<T*>(instance)->*member)();
    }

    template <typename T>
    typename boost::enable_if<boost::is_base_of<Actor, T>, AnyFunction>::type
        getStrandAccessor()
    {
      return AnyFunction::from(boost::function<qi::Strand*(void*)>(
          boost::bind(&callWithVoid<T>, &T::strand, _1)));
    }
    template <typename T>
    typename boost::disable_if<boost::is_base_of<Actor, T>, AnyFunction>::type
        getStrandAccessor()
    {
      return AnyFunction();
    }
  }

  template<typename T> void ObjectTypeBuilderBase::buildFor(bool autoRegister)
  {
    // We are erasing T here: we must pass everything the builder needs to know about:
    // - typeid
    // - cloner/deleter
    // - serializer, ...
    // => we need all TypeInterface* methods, but we do not want another TypeInterface*
    // to answer to typeOf<T>
    static DefaultTypeImpl<T> implTypeInterface;
    xBuildFor(&implTypeInterface, autoRegister, detail::getStrandAccessor<T>());

    if (std::is_base_of<Actor, T>::value)
      setThreadingModel(ObjectThreadingModel_SingleThread);
    else
      setThreadingModel(ObjectThreadingModel_MultiThread);
  }

  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilderBase::advertiseMethod(const std::string& name,
                                                      FUNCTION_TYPE function,
                                                      MetaCallType threadingModel,
                                                      int id)
  {
    MetaMethodBuilder builder;
    AnyFunction f = AnyFunction::from(function);
    builder.setName(name);
    builder.setSignature(f);

    // throw on error
    return xAdvertiseMethod(builder, f, threadingModel, id);
  }

  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilderBase::advertiseMethod(MetaMethodBuilder& builder,
                                                      FUNCTION_TYPE function,
                                                      MetaCallType threadingModel,
                                                      int id)
  {
    AnyFunction f = AnyFunction::from(function);
    builder.setSignature(f);

    // throw on error
    return xAdvertiseMethod(builder, f, threadingModel, id);
  }

  template<typename U>
  void ObjectTypeBuilderBase::inherits(std::ptrdiff_t offset)
  {
    return inherits(typeOf<
      typename boost::remove_reference<U>::type>(), offset);
  }

  template<typename T>
  template<typename U>
  void ObjectTypeBuilder<T>::inherits()
  {
    qiLogCategory("qitype.objectbuilder");
    // Compute the offset between T and U
    T* ptr = reinterpret_cast<T*>(0x10000);
    U* pptr = ptr;
    std::ptrdiff_t offset = reinterpret_cast<intptr_t>(pptr) - reinterpret_cast<intptr_t>(ptr);
    qiLogDebug() << "Offset check T(" << qi::typeIdRuntime(ptr).name() << ")= " << ptr
                 << ", U(" << qi::typeIdRuntime(pptr).name() << ")= " << pptr << ", T-U= " << offset;
    return ObjectTypeBuilderBase::inherits<U>(offset);
  }

  namespace detail
  {
    template <typename F, typename T>
    void checkRegisterParent(ObjectTypeBuilder<T>& builder)
    {
      if constexpr (std::is_member_function_pointer_v<F>) {
        using ClassType = boost::callable_traits::class_of_t<F>;
        builder.template inherits<ClassType>();
      }
    }
  };

  template <typename T>
  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilder<T>::advertiseMethod(const std::string& name, FUNCTION_TYPE function, MetaCallType threadingModel, int id)
  {
    // Intercept advertise to auto-register parent type if this is a parent method
    // Note: if FUNCTION_TYPE is a grandparent method, we will incorrectly add it
    // as a child
    detail::checkRegisterParent<FUNCTION_TYPE>(*this);

    // throw on error
    return ObjectTypeBuilderBase::advertiseMethod(name, function, threadingModel, id);
  }

  template <typename T>
  template <typename FUNCTION_TYPE>
  unsigned int ObjectTypeBuilder<T>::advertiseMethod(MetaMethodBuilder& name, FUNCTION_TYPE function, MetaCallType threadingModel, int id)
  {
    // Intercept advertise to auto-register parent type if this is a parent method
    // Note: if FUNCTION_TYPE is a grandparent method, we will incorrectly add it
    // as a child
    detail::checkRegisterParent<FUNCTION_TYPE>(*this);

    // throw on error
    return ObjectTypeBuilderBase::advertiseMethod(name, function, threadingModel, id);
  }

  template <typename T>
  AnyObject ObjectTypeBuilder<T>::object(T* ptr, boost::function<void (GenericObject*)> onDestroy)
  {
    return ObjectTypeBuilderBase::object(static_cast<void*>(ptr), onDestroy);
  }

  template<typename T>
  void ObjectTypeBuilder<T>::registerType()
  {
    ::qi::registerType(qi::typeId<T>(), type());
  }

  template<typename A>
  unsigned int
  ObjectTypeBuilderBase::advertiseSignal(const std::string& eventName, A accessor, int id, bool isSignalProperty)
  {
    auto fun = [accessor = std::move(accessor)](void* instance) mutable {
      return &detail::accessor::invoke(accessor, instance);
    };
    using ValueType = detail::accessor::ValueType<A>;
    using FunctionType = typename ValueType::FunctionType;
    return xAdvertiseSignal(eventName, detail::functionArgumentsSignature<FunctionType>(), fun, id, isSignalProperty);
  }

  template <typename A>
  unsigned int ObjectTypeBuilderBase::advertiseProperty(const std::string& name, A accessor)
  {
    unsigned int id = advertiseSignal(name, accessor, -1, true);
    auto fun = [accessor = std::move(accessor)](void* instance) mutable {
      return &detail::accessor::invoke(accessor, instance);
    };
    using ValueType = detail::accessor::ValueType<A>;
    using PropertyType = typename ValueType::PropertyType;
    return xAdvertiseProperty(name, typeOf<PropertyType>()->signature(), fun, id);
  }

  template <typename T> unsigned int ObjectTypeBuilderBase::advertiseSignal(const std::string& name, SignalMemberGetter getter, int id, bool isSignalProperty)
  {
    return xAdvertiseSignal(name, detail::functionArgumentsSignature<T>(), getter, id, isSignalProperty);
  }

  template<typename T>
  inline unsigned int ObjectTypeBuilderBase::advertiseProperty(const std::string& eventName, PropertyMemberGetter getter)
  {
    return xAdvertiseProperty(eventName, typeOf<T>()->signature(), getter);
  }

  template<typename T>
  unsigned int
  ObjectTypeBuilderBase::advertiseId(const std::string& name, T element)
  {
    // Most properties are also signals, but signals are not properties.
    // Therefore we must check if the function is a property accessor before
    // checking if it is a signal accessor, otherwise we might advertise a
    // property as a signal only.
    constexpr auto isPropertyAccessor = detail::accessor::IsAccessor<T>() &&
        std::is_base_of_v<qi::PropertyBase, detail::accessor::ValueType<T>>;
    [[maybe_unused]]
    constexpr auto interfaceMarker = std::string_view("_interface_");

    if constexpr (isPropertyAccessor) {
      std::string_view nameView(name);
      if (boost::algorithm::starts_with(nameView, interfaceMarker)) {
        nameView.remove_prefix(interfaceMarker.size());
      }
      return advertiseProperty(std::string(nameView), std::move(element));
    }
    else {
      constexpr auto isSignalAccessor = detail::accessor::IsAccessor<T>() &&
          std::is_base_of_v<qi::SignalBase, detail::accessor::ValueType<T>>;
      if constexpr (isSignalAccessor) {
        std::string_view nameView(name);
        if (boost::algorithm::starts_with(nameView, interfaceMarker)) {
          nameView.remove_prefix(interfaceMarker.size());
        }
        return advertiseSignal(std::string(nameView), std::move(element));
      }
      else {
        return advertiseMethod(name, std::move(element));
      }
    }
  }

  template<typename T>
  ObjectTypeBuilderBase&
  ObjectTypeBuilderBase::advertise(const std::string& name, T element)
  {
    advertiseId(name, std::move(element));
    return *this;
  }

KA_WARNING_POP()

}


#endif  // _QITYPE_DETAIL_OBJECTTYPEBUILDER_HXX_

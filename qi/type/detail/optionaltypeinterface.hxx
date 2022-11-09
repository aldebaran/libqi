/*
**  Copyright (C) 2017 SoftBank Robotics
**  See COPYING for the license
*/

#ifndef QITYPE_DETAIL_OPTIONALTYPEINTERFACE_HXX
#define QITYPE_DETAIL_OPTIONALTYPEINTERFACE_HXX

#include <qi/type/typeinterface.hpp>
#include <qi/type/detail/typeimpl.hxx>
#include <boost/optional.hpp>

#if defined(__cpp_lib_optional) && defined(__has_include)
#  if __has_include(<optional>)
#    include <optional>
#    define QI_HAS_STD_OPTIONAL 1
#  else
#    define QI_HAS_STD_OPTIONAL 0
#  endif
#else
#  define QI_HAS_STD_OPTIONAL 0
#endif

namespace qi
{
  /// With Any T:
  ///   (boost::optional<T> || std::optional<T>) O
  template<typename O>
  class OptionalTypeInterfaceImpl: public OptionalTypeInterface
  {
  public:
    using ValueType = typename O::value_type;

    OptionalTypeInterfaceImpl();
    TypeInterface* valueType() override;
    bool hasValue(void* storage) override;
    AnyReference value(void* storage) override;
    void set(void** storage, void* valueStorage) override;
    void reset(void** storage) override;

    using TypeMethodsImpl = DefaultTypeImplMethods<O, TypeByPointerPOD<O>>;
    _QI_BOUNCE_TYPE_METHODS(TypeMethodsImpl)
    TypeInterface* _valueType;
  };

  template<typename O>
  OptionalTypeInterfaceImpl<O>::OptionalTypeInterfaceImpl()
  {
    _valueType = typeOf<ValueType>();
  }

  template<typename O>
  TypeInterface* OptionalTypeInterfaceImpl<O>::valueType()
  {
    return _valueType;
  }

  template<typename O>
  bool OptionalTypeInterfaceImpl<O>::hasValue(void* storage)
  {
    auto& opt = *reinterpret_cast<O*>(ptrFromStorage(&storage));
    return static_cast<bool>(opt);
  }

  template<typename O>
  AnyReference OptionalTypeInterfaceImpl<O>::value(void* storage)
  {
    auto& opt = *reinterpret_cast<O*>(ptrFromStorage(&storage));
    if (!opt)
      return AnyReference(typeOf<void>());
    auto& value = opt.value(); // value must be a reference type.
    return AnyReference::from(value);
  }

  template<typename O>
  void OptionalTypeInterfaceImpl<O>::set(void** storage, void* valueStorage)
  {
    auto& opt = *reinterpret_cast<O*>(ptrFromStorage(storage));
    opt = *reinterpret_cast<ValueType*>(_valueType->ptrFromStorage(&valueStorage));
  }

  template<typename O>
  void OptionalTypeInterfaceImpl<O>::reset(void** storage)
  {
    auto& opt = *reinterpret_cast<O*>(ptrFromStorage(storage));
    opt = O{};
  }

  template <typename T>
  class TypeImpl<boost::optional<T>> : public OptionalTypeInterfaceImpl<boost::optional<T>> {};
#if QI_HAS_STD_OPTIONAL
  template<typename T>
  class TypeImpl<std::optional<T>>: public OptionalTypeInterfaceImpl<std::optional<T>>{};
#endif

  template<> class TypeImpl<boost::none_t>: public TypeImpl<void> {};
#if QI_HAS_STD_OPTIONAL
  template<> class TypeImpl<std::nullopt_t>: public TypeImpl<void> {};
#endif
}

#endif  // QITYPE_DETAIL_OPTIONALTYPEINTERFACE_HXX

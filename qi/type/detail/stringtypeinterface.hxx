#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_TYPESTRING_HXX_
#define _QITYPE_DETAIL_TYPESTRING_HXX_

#include <algorithm>
#include <qi/os.hpp>
#include <qi/type/detail/structtypeinterface.hxx>

namespace qi
{
  inline std::string StringTypeInterface::getString(void* storage)
  {
    ManagedRawString raw = get(storage);
    std::string res(raw.first.first, raw.first.second);
    if (raw.second)
      raw.second(raw.first);
    return res;
  }

  inline void StringTypeInterface::set(void** storage, const std::string& val)
  {
    set(storage, val.c_str(), val.size());
  }

  class QI_API StringTypeInterfaceImpl: public StringTypeInterface
  {
  public:
    using Methods = DefaultTypeImplMethods<std::string, TypeByPointerPOD<std::string>>;
    ManagedRawString get(void* storage) override
    {
      std::string* ptr = (std::string*)Methods::ptrFromStorage(&storage);
      return ManagedRawString(RawString(const_cast<char*>(ptr->c_str()), ptr->size()),
          Deleter());
    }
    void set(void** storage, const char* value, size_t sz) override
    {
      std::string* ptr = (std::string*)Methods::ptrFromStorage(storage);
      ptr->assign(value, sz);
    }

    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  template<>
  class TypeImpl<std::string>: public StringTypeInterfaceImpl
  {};

  class QI_API TypeCStringImpl: public StringTypeInterface
  {
  public:
    ManagedRawString get(void* storage) override
    {
      return ManagedRawString(RawString((char*)storage, strlen((char*)storage)),
          Deleter());
    }
    void set(void** storage, const char* ptr, size_t /*sz*/) override
    {
      *(char**)storage = qi::os::strdup(ptr);
    }
    void* clone(void* src) override
    {
      return qi::os::strdup((char*)src);
    }
    void destroy(void* src) override
    {
      free(src);
    }
    using Methods = DefaultTypeImplMethods<char*, TypeByValue<char*>>;
    _QI_BOUNCE_TYPE_METHODS_NOCLONE(Methods);
  };

  template<>
  class TypeImpl<char*>: public TypeCStringImpl
  {};


  template<int I> class TypeImpl<char [I]>: public StringTypeInterface
  {
  public:
    void* clone(void* src) override
    {
      char* res = new char[I];
      memcpy(res, src, I);
      return res;
    }
    void destroy(void* ptr) override
    {
      delete[]  (char*)ptr;
    }
    ManagedRawString get(void* storage) override
    {
      return ManagedRawString(RawString((char*)storage, I-1),
          Deleter());
    }
    void set(void** /*storage*/, const char* /*ptr*/, size_t /*sz*/) override
    {
      qiLogCategory("qitype.typestring");
      // haha...no
      qiLogWarning() << "set on C array not implemented";
    }

    using Methods = DefaultTypeImplMethods<char[I], TypeByPointerPOD<char[I]>>;
      _QI_BOUNCE_TYPE_METHODS_NOCLONE(Methods);
  };

  inline StringTypeInterface::ManagedRawString makeManagedString(const std::string& s)
  {
    return StringTypeInterface::ManagedRawString(StringTypeInterface::RawString(const_cast<char*>(s.c_str()), s.size()),
                                                 StringTypeInterface::Deleter());
  }

  inline StringTypeInterface::ManagedRawString makeManagedString(std::string&& s)
  {
    // Move the string parameter in a new allocated string. This way, we avoid string buffer copy.
    const auto ms = new auto(std::move(s));

    return StringTypeInterface::ManagedRawString(StringTypeInterface::RawString(const_cast<char*>(ms->c_str()), ms->size()),
                                                 // Capture ms and delete it. No action is performed on RawString
                                                 [=](const StringTypeInterface::RawString&){ delete ms; });
  }

  namespace detail
  {
    template<typename T>
    T constructFromStr(const std::string& str)
    {
      return T(str);
    }
  }

  /// Declare a Type for T of Kind string.
  ///
  /// T must be default-constructible, copy-constructible.
  /// Function<std::string (T)> ToStr
  /// Function<T (std::string)> FromStr
  template<typename T, typename ToStr, typename FromStr>
  class TypeEquivalentString: public StringTypeInterface
  {
  public:
    TypeEquivalentString(ToStr toStr, FromStr fromStr)
      : _toStr(std::move(toStr))
      , _fromStr(std::move(fromStr))
    {}

    using Impl = DefaultTypeImplMethods<T, TypeByPointerPOD<T>>;

    void set(void** storage, const char* ptr, size_t sz) override
    {
      auto* const inst = reinterpret_cast<T*>(ptrFromStorage(storage));
      *inst = _fromStr(std::string(ptr, sz));
    }

    ManagedRawString get(void* storage) override
    {
      auto* const ptr = reinterpret_cast<T*>(Impl::ptrFromStorage(&storage));
      return makeManagedString(std::bind(_toStr, std::ref(*ptr))());
    }

    _QI_BOUNCE_TYPE_METHODS(Impl);
    ToStr _toStr;
    FromStr _fromStr;
  };

  /// T must be default-constructible, copy-constructible.
  /// Function<std::string (T)> ToStr
  /// Function<T (std::string)> FromStr
  template<typename T, typename ToStr, typename FromStr = T (*)(const std::string&)>
  StringTypeInterface* makeTypeEquivalentString(ToStr&& toStr,
                                                FromStr&& fromStr = &detail::constructFromStr<T>)
  {
    return new TypeEquivalentString<T, ka::Decay<ToStr>, ka::Decay<FromStr>>(
      std::forward<ToStr>(toStr), std::forward<FromStr>(fromStr));
  }
}

/** Register type \p in the type system as string kind, using constructor
 * for setter, and function \p tostr for getter
 */
#define QI_EQUIVALENT_STRING_REGISTER(type, tostr) \
  static bool BOOST_PP_CAT(__qi_registration, __COUNTER__) QI_ATTR_UNUSED \
    = qi::registerType(qi::typeId<type>(), \
                       qi::makeTypeEquivalentString<type>(tostr))

/** Register type \p in the type system as string kind, using \p tostr for getter, and
 * \p fromstr for setter.
 */
#define QI_EQUIVALENT_STRING_REGISTER2(type, tostr, fromstr) \
  static bool BOOST_PP_CAT(__qi_registration, __COUNTER__) QI_ATTR_UNUSED \
    = qi::registerType(qi::typeId<type>(), \
                       qi::makeTypeEquivalentString<type>(tostr, fromstr))


#endif  // _QITYPE_DETAIL_TYPESTRING_HXX_

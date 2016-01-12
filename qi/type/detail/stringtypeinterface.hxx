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
      return ManagedRawString(RawString((char*)ptr->c_str(), ptr->size()),
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
    void set(void** storage, const char* ptr, size_t sz) override
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
    void set(void** storage, const char* ptr, size_t sz) override
    {
      qiLogCategory("qitype.typestring");
      // haha...no
      qiLogWarning() << "set on C array not implemented";
    }

    using Methods = DefaultTypeImplMethods<char[I], TypeByPointerPOD<char[I]>>;
      _QI_BOUNCE_TYPE_METHODS_NOCLONE(Methods);
  };

  template<class Func, class... Args >
  auto callWithInstance(Func&& f, Args&&... args)
    -> decltype(std::forward<Func>(f)(std::forward<Args>(args)...))
  {
    return std::forward<Func>(f)(std::forward<Args>(args)...);
  }

  template<class Func, class Obj, class... Args >
  auto callWithInstance(Func&& f, Obj&& o, Args&&... args)
  -> decltype((std::forward<Obj>(o).*std::forward<Func>(f))(std::forward<Args>(args)...))
  {
    return (std::forward<Obj>(o).*std::forward<Func>(f))(std::forward<Args>(args)...);
  }

  inline StringTypeInterface::ManagedRawString makeManagedString(const std::string& s)
  {
    return StringTypeInterface::ManagedRawString(StringTypeInterface::RawString((char*)s.c_str(), s.size()),
                                                 StringTypeInterface::Deleter());
  }

  inline StringTypeInterface::ManagedRawString makeManagedString(std::string&& s)
  {
    const auto size = s.size() + 1;
    auto strKeptAlive = new char[size]();
    std::copy(begin(s), end(s), strKeptAlive);
    return StringTypeInterface::ManagedRawString(StringTypeInterface::RawString(strKeptAlive, size),
                                                 [](const StringTypeInterface::RawString& rawStr){ delete[] rawStr.first; });
  }

  /** Declare a Type for T of Kind string.
  *
  * T must be default-constructible, copy-constructible, and
  * provide a constructor accepting a string.
  * F must be a member function pointer, member object pointer, or free function
  * returning a const string&.
  */
  template<typename T, typename F> class TypeEquivalentString: public StringTypeInterface
  {
  public:
    TypeEquivalentString(F f): _getter(f) {}
    using Impl = DefaultTypeImplMethods<T, TypeByPointerPOD<T>>;

    void set(void** storage, const char* ptr, size_t sz) override
    {
      T* inst = (T*)ptrFromStorage(storage);
      *inst = T(std::string(ptr, sz));
    }

    ManagedRawString get(void* storage) override
    {
      T* ptr = (T*)Impl::ptrFromStorage(&storage);
      return makeManagedString(callWithInstance(_getter, *ptr));
    }

    _QI_BOUNCE_TYPE_METHODS(Impl);
    F _getter;
  };

  template<typename T, typename F>
  StringTypeInterface* makeTypeEquivalentString(T*, F f)
  {
    return new TypeEquivalentString<T, F>(f);
  }
}

/** Register type \p type in the type system as string kind, using constructor
 * for setter, and function \p func for getter
 */
#define QI_EQUIVALENT_STRING_REGISTER(type, func) \
  static bool BOOST_PP_CAT(__qi_registration, __COUNTER__) \
    = qi::registerType(typeid(type),  qi::makeTypeEquivalentString((type*)0, func))

#endif  // _QITYPE_DETAIL_TYPESTRING_HXX_

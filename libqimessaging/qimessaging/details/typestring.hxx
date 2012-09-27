#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPESTRING_HXX_
#define _QIMESSAGING_TYPESTRING_HXX_

namespace qi
{
  class QIMESSAGING_API TypeStringImpl: public TypeString
  {
  public:
    typedef DefaultTypeImplMethods<std::string,
    TypeDefaultAccess<std::string>,
    TypeDefaultClone<TypeDefaultAccess<std::string> >,
    TypeDefaultValue<TypeDefaultAccess<std::string> >,
    TypeDefaultSerialize<TypeDefaultAccess<std::string> >
    > Methods;
    virtual std::string get(void* storage) const
    {
      std::string* ptr = (std::string*)Methods::ptrFromStorage(&storage);
      return *ptr;
    }
    virtual void set(void** storage, const std::string& value)
    {
      std::string* ptr = (std::string*)Methods::ptrFromStorage(storage);
      *ptr = value;
    }

    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  template<> class TypeImpl<std::string>: public TypeStringImpl{};

  class CStringClone
  {
  public:
    static void* clone(void* src)
    {
      return strdup((char*)src);
    }
    static void destroy(void* src)
    {
      free(src);
    }
  };

  class QIMESSAGING_API TypeCStringImpl: public TypeString
  {
  public:
    virtual std::string get(void* storage) const
    {
      return std::string((char*)storage);
    }
    virtual void set(void** storage, const std::string& str)
    {
      *(char**)storage = strdup(str.c_str());
    }

    typedef DefaultTypeImplMethods<char*, TypeDirectAccess<char*>, CStringClone> TypeMethodsImpl;
    _QI_BOUNCE_TYPE_METHODS(TypeMethodsImpl);
  };

  template<> class TypeImpl<char*>: public TypeCStringImpl{};

  template<int I>
  class TypeCArrayClone
  {
  public:
    static void* clone(void* src)
    {
      char* res = new char[I];
      memcpy(res, src, I);
      return res;
    }
    static void destroy(void* ptr)
    {
      delete[]  (char*)ptr;
    }
  };
template<int I>
  class TypeCArrayValue
  {
  public:
    static bool toValue(const void* ptr, detail::DynamicValue& val)
    {
      val = std::string((const char*)ptr, I-1);
      return true;
    }
    static void* fromValue(const detail::DynamicValue& val)
    {
      std::string s = val.toString();
      if (s.length() != I)
      {
        qiLogError("Type") << "C string cast fail between char["
                               << I  <<"] and " << s;
        return 0;
      }
      char* res = new char[I];
      memcpy(res, s.c_str(), I);
      return res;
    }
  };

  template<int I> class TypeCArraySerialize
  {
  public:
    static void  serialize(ODataStream& s, const void* ptr)
    {
      s << (const char*)ptr;
    }
    static void* deserialize(IDataStream& s)
    {
      std::string str;
      s >> str;
      if (str.length() >= I)
        return 0;
      char* res = new char[I];
      strncpy(res, str.c_str(), str.length());
      return res;
    }
    static std::string signature()
    {
      return signatureFromType<std::string>::value();
    }
  };

  template<int I> class TypeImpl<char [I]>: public TypeString
  {
  public:
    virtual std::string get(void* storage) const
    {
      char* ptr = (char*) storage;
      return ptr;
    }
    virtual void set(void** storage, const std::string& value)
    {
      // haha...no
      qiLogWarning("qi.meta") << "set on C array not implemented";
    }

    typedef  DefaultTypeImplMethods<char[I],
      TypeDefaultAccess<char[I]>,
      TypeCArrayClone<I>,
      TypeCArrayValue<I>,
      TypeCArraySerialize<I>
      > Methods;
      _QI_BOUNCE_TYPE_METHODS(Methods);
  };
}

QI_REGISTER_MAPPING("s", std::string);
#endif

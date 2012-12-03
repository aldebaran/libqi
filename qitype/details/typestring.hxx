#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPESTRING_HXX_
#define _QITYPE_DETAILS_TYPESTRING_HXX_

namespace qi
{
  inline std::string TypeString::getString(void* storage) const
  {
    std::pair<char*, size_t> res = get(storage);
    return std::string(res.first, res.second);
  }
  inline void TypeString::set(void** storage, const std::string& val)
  {
    set(storage, val.c_str(), val.size());
  }
  class QITYPE_API TypeStringImpl: public TypeString
  {
  public:
    typedef DefaultTypeImplMethods<std::string,
      detail::TypeImplMethodsBySize<std::string>::type
    > Methods;
    virtual std::pair<char*, size_t> get(void* storage) const
    {
      std::string* ptr = (std::string*)Methods::ptrFromStorage(&storage);
      return std::make_pair((char*)ptr->c_str(), ptr->size());
    }
    virtual void set(void** storage, const char* value, size_t sz)
    {
      std::string* ptr = (std::string*)Methods::ptrFromStorage(storage);
      ptr->assign(value, sz);
    }

    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  template<> class TypeImpl<std::string>: public TypeStringImpl{};

  class QITYPE_API TypeCStringImpl: public TypeString
  {
  public:
    virtual std::pair<char*, size_t> get(void* storage) const
    {
      return std::make_pair((char*)storage, strlen((char*)storage));
    }
    virtual void set(void** storage, const char* ptr, size_t sz)
    {
      *(char**)storage = qi::os::strdup(ptr);
    }
    virtual void* clone(void* src)
    {
      return strdup((char*)src);
    }
    virtual void destroy(void* src)
    {
      free(src);
    }
    typedef DefaultTypeImplMethods<char*, TypeByValue<char*> > Methods;
    _QI_BOUNCE_TYPE_METHODS_NOCLONE(Methods);
  };

  template<> class TypeImpl<char*>: public TypeCStringImpl{};


  template<int I> class TypeImpl<char [I]>: public TypeString
  {
  public:
    virtual void* clone(void* src)
    {
      char* res = new char[I];
      memcpy(res, src, I);
      return res;
    }
    virtual void destroy(void* ptr)
    {
      delete[]  (char*)ptr;
    }
    virtual std::pair<char*, size_t> get(void* storage) const
    {
      return std::make_pair((char*)storage, I-1);
    }
    virtual void set(void** storage, const char* ptr, size_t sz)
    {
      // haha...no
      qiLogWarning("qi.meta") << "set on C array not implemented";
    }

    typedef  DefaultTypeImplMethods<char[I],
      TypeByPointer<char[I]> > Methods;
      _QI_BOUNCE_TYPE_METHODS_NOCLONE(Methods);
  };
}

#endif  // _QITYPE_DETAILS_TYPESTRING_HXX_

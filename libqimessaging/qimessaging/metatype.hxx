/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once

#ifndef _QI_MESSAGING_METATYPE_HXX_
#define _QI_MESSAGING_METATYPE_HXX_

#include <cstring>
/* This file contains the default-provided MetaType specialisations
 *
 */

namespace qi  {
/* C array. We badly need this because the type of literal string "foo"
* is char[4] not char*
*
*/
template<int I>
class MetaTypeCArrayClone
{
  public:
  void* clone(void* src)
  {
    char* res = new char[I];
    memcpy(res, src, I);
    return res;
  }
  void destroy(void* ptr)
  {
    delete[]  (char*)ptr;
  }
};

template<int I>
class MetaTypeCArrayValue
{
public:
  bool toValue(const void* ptr, detail::Value& val)
  {
    val = std::string((const char*)ptr, I-1);
    return true;
  }
  void* fromValue(const detail::Value& val)
  {
    std::string s = val.toString();
    if (s.length() != I)
    {
      qiLogError("MetaType") << "C string cast fail between char["
        << I  <<"] and " << s;
      return 0;
    }
    char* res = new char[I];
    memcpy(res, s.c_str(), I);
    return res;
  }
};

template<int I> class MetaTypeCArraySerialize
{
public:
  void  serialize(ODataStream& s, const void* ptr)
  {
    s << (const char*)ptr;
  }
  void* deserialize(IDataStream& s)
  {
    std::string str;
    s >> str;
    if (str.length() >= I)
      return 0;
    char* res = new char[I];
    strncpy(res, str.c_str(), str.length());
    return res;
  }
};

template<int I> class MetaTypeImpl<char [I]>
  :public  DefaultMetaTypeImpl<char[I],
    MetaTypeCArrayClone<I>,
    MetaTypeCArrayValue<I>,
    MetaTypeCArraySerialize<I>
    >{};

}
#endif

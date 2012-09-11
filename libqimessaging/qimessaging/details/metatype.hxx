/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once

#ifndef _QI_MESSAGING_METATYPE_HXX_
#define _QI_MESSAGING_METATYPE_HXX_

#include <qi/types.hpp>
#include <cstring>
#include <map>
#include <vector>
#include <list>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>

/* This file contains the default-provided MetaType specialisations
 *
 */

/** Integral types.
 * Since long is neither int32 nor uint32 on 32 bit platforms,
 * use all known native types instead of size/signedness explicit
 * types.
 */
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(char);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(signed char);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(unsigned char);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(short);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(unsigned short);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(int);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(unsigned int);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(long);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(unsigned long);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(long long);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(unsigned long long);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(float);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(double);
QI_METATYPE_CONVERTIBLE_SERIALIZABLE(std::string);

#define _CONTAINER(c) namespace qi {             \
  template<typename T> class MetaTypeImpl<c<T> >:  \
  public DefaultMetaTypeImpl<c<T>,               \
  MetaTypeDefaultClone<c<T> >,                   \
  MetaTypeDefaultValue<c<T> >,                   \
  MetaTypeDefaultSerialize<c<T> >                \
  > {}; }

_CONTAINER(std::list);
_CONTAINER(std::vector);

#undef _CONTAINER

#define _MAP(c) namespace qi {                                \
  template<typename K, typename V> class MetaTypeImpl<c<K,V> >:  \
  public DefaultMetaTypeImpl<c<K,V>,               \
  MetaTypeDefaultClone<c<K,V> >,                   \
  MetaTypeDefaultValue<c<K,V> >,                   \
  MetaTypeDefaultSerialize<c<K,V> >                \
  > {}; }

_MAP(std::map);
#undef _MAP

namespace qi {
  // void
  template<> class MetaTypeImpl<void>: public MetaType
  {
  public:
    const std::type_info& info()
    {
      return typeid(void);
    }
    std::string signature()                  { return "\0"; }
    void* clone(void*)                       { return 0;}
    void destroy(void* ptr)                  {}
    bool toValue(const void *ptr, detail::Value & v) {v.clear(); return true;}
    void* fromValue(const detail::Value & v) { return 0;}
    void serialize(ODataStream&, const void*){}
    void* deserialize(IDataStream&)          { return 0;}
  };

  //reference

  template<typename T> class MetaTypeImpl<T&>
      : public MetaTypeImpl<T> {};
}

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
    std::string signature()
    {
      return signatureFromType<std::string>::value();
    }
  };

  template<int I> class MetaTypeImpl<char [I]>
      :public  DefaultMetaTypeImpl<char[I],
      MetaTypeCArrayClone<I>,
      MetaTypeCArrayValue<I>,
      MetaTypeCArraySerialize<I>
      >{};


  namespace detail {
    struct signature_function_arg_apply {
      signature_function_arg_apply(std::ostream* val)
        : val(*val)
      {}

      template<typename T> void operator()(T *x) {
        val << qi::metaTypeOf<T>()->signature();
      }

      std::ostream &val;
    };

    template<typename T> inline
    std::string functionArgumentsSignature()
    {
      std::stringstream sigs;

      typedef typename boost::function_types::parameter_types<T>::type ArgsType;
      boost::mpl::for_each<
      boost::mpl::transform_view<ArgsType,
      boost::add_pointer<
        boost::remove_const<
        boost::remove_reference<boost::mpl::_1> > > > > (qi::detail::signature_function_arg_apply(&sigs));

      return sigs.str();
    }

  }

}
#endif

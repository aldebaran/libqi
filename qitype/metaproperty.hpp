#pragma once
#ifndef _QITYPE_METAPROPERTY_HPP_
#define _QITYPE_METAPROPERTY_HPP_

#include <qitype/type.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi
{
  class QITYPE_API MetaProperty
  {
  public:
    MetaProperty(unsigned int uid, const std::string& name, const qi::Signature& sig);
    MetaProperty();
    const std::string& name() const;
    const qi::Signature& signature() const;
    std::string toString() const;
    unsigned int uid() const;

  private:
    unsigned int  _uid;
    std::string   _name;
    qi::Signature _signature;
    QI_TYPE_STRUCT_PRIVATE_ACCESS(MetaProperty);
  };
}


QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::MetaProperty,
  ("uid",_uid),
  ("name",_name),
  ("signature", _signature));


#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_METAPROPERTY_HPP_

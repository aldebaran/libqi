#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_METAPROPERTY_HPP_
#define _QI_TYPE_METAPROPERTY_HPP_

#include <qi/type/typeinterface.hpp>
#include <ka/macro.hpp>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )

namespace qi
{
  class QI_API MetaProperty
  {
  public:
    MetaProperty(unsigned int uid, const std::string& name, const qi::Signature& sig);
    MetaProperty();
    const std::string& name() const;
    const qi::Signature& signature() const;
    std::string toString() const;
    unsigned int uid() const;

    /** return true if property is considered internal, and should not be listed
     */
    bool isPrivate() const;

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


KA_WARNING_POP()

#endif  // _QITYPE_METAPROPERTY_HPP_

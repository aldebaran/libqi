#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_METAMETHOD_HPP_
#define _QITYPE_METAMETHOD_HPP_

#include <qitype/api.hpp>
#include <string>
#include <qitype/type.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  /// Representation of a method in an GenericObject.
  class QITYPE_API MetaMethod {
  public:
    MetaMethod()
      : _uid(0)
    {}

    MetaMethod(unsigned int uid, const std::string& sigret, const std::string& signature);

    std::string signature() const;
    std::string sigreturn() const;

    unsigned int       uid() const;

  private:
    unsigned int      _uid;
    // C4251
    std::string       _signature;
    // C4251
    std::string       _sigreturn;
    friend class TypeImpl<MetaMethod>;
  };

} // namespace qi

QI_TYPE_STRUCT(qi::MetaMethod, _signature, _sigreturn, _uid);

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_METAMETHOD_HPP_

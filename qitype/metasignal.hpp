#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_METASIGNAL_HPP_
#define _QITYPE_METASIGNAL_HPP_

#include <qitype/type.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  /// Representation of a Signal in an GenericObject.
  class QITYPE_API MetaSignal {
  public:
    MetaSignal(unsigned int uid, const std::string &name, const std::string &sig);
    MetaSignal();
    ~MetaSignal();

    const std::string &name() const;
    std::string toString() const;  // name::(sig)
    const std::string &parametersSignature() const;
    unsigned int       uid() const;

  private:
    unsigned int _uid;
    // C4251
    std::string  _name;
    std::string  _signature;
    QI_TYPE_STRUCT_PRIVATE_ACCESS(MetaSignal);
  };

}; // namespace qi

QI_TYPE_STRUCT(qi::MetaSignal, _uid, _name, _signature);

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_METASIGNAL_HPP_

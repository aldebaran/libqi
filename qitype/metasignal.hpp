#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_METASIGNAL_HPP_
#define _QITYPE_METASIGNAL_HPP_

#include <qitype/signature.hpp>
#include <qitype/type.hpp>

namespace qi {


  /// Representation of a Signal in an GenericObject.
  class QITYPE_API MetaSignal {
  public:
    MetaSignal(unsigned int uid, const std::string &sig);
    MetaSignal();
    ~MetaSignal();

    const std::string &signature() const;
    unsigned int       uid() const;

  private:
    unsigned int _uid;
    std::string  _signature;
    QI_TYPE_STRUCT_PRIVATE_ACCESS(MetaSignal);
  };

}; // namespace qi

QI_TYPE_STRUCT(qi::MetaSignal, _uid, _signature);

#endif  // _QITYPE_METASIGNAL_HPP_

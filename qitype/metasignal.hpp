#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_METASIGNAL_HPP_
#define _QIMESSAGING_METASIGNAL_HPP_

#include <qitype/signature.hpp>

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
    friend class TypeImpl<MetaSignal>;
  };

}; // namespace qi

QI_TYPE_STRUCT(qi::MetaSignal, _uid, _signature);

#endif  // _QIMESSAGING_METASIGNAL_HPP_

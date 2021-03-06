#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_METASIGNAL_HPP_
#define _QI_TYPE_METASIGNAL_HPP_

#include <qi/type/typeinterface.hpp>
#include <ka/macro.hpp>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )

namespace qi {

  /// Representation of a Signal in an GenericObject.
  class QI_API MetaSignal {
  public:
    MetaSignal(unsigned int uid, const std::string &name, const qi::Signature &sig);
    MetaSignal();
    ~MetaSignal();

    const std::string &name() const;
    std::string        toString() const;  // name::(sig)
    const Signature   &parametersSignature() const;
    unsigned int       uid() const;

    /** return true if signal is considered internal, and should not be listed
     */
    bool               isPrivate() const;

  private:
    unsigned int  _uid;
    // C4251
    std::string   _name;
    qi::Signature _signature;
    QI_TYPE_STRUCT_PRIVATE_ACCESS(MetaSignal);
  };

}; // namespace qi

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::MetaSignal,
  ("uid",_uid),
  ("name",_name),
  ("signature", _signature));

KA_WARNING_POP()

#endif  // _QITYPE_METASIGNAL_HPP_

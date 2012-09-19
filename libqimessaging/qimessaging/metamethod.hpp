#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_METAMETHOD_HPP_
#define _QIMESSAGING_METAMETHOD_HPP_

#include <qimessaging/metafunction.hpp>
#include <qimessaging/methodtype.hpp>


namespace qi {

  /// Representation of a method in an GenericObject.
  class QIMESSAGING_API MetaMethod {
  public:
    MetaMethod() {};
    MetaMethod(unsigned int uid, const std::string& sigret, const std::string& signature);

    std::string signature() const;
    std::string sigreturn() const;

    unsigned int       uid() const;

  protected:
    unsigned int      _uid;
    std::string       _signature;
    std::string       _sigreturn;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaMethod &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaMethod &meta);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaMethod &meta);

}; // namespace qi


#endif  // _QIMESSAGING_METAMETHOD_HPP_

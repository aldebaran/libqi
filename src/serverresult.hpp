#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVERRESULT_HPP_
#define _SRC_SERVERRESULT_HPP_

#include <boost/shared_ptr.hpp>
#include <qi/future.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/binaryencoder.hpp>
#include <qimessaging/transportsocket.hpp>

namespace qi {

  inline void serverResultAdapter(qi::Future<GenericValuePtr> future, TransportSocketPtr socket, const qi::MessageAddress &replyaddr) {
    qi::Message ret(Message::Type_Reply, replyaddr);
    qi::Buffer      result;
    qi::BinaryEncoder ods(result);

    if (future.hasError()) {
      ret.setType(qi::Message::Type_Error);
      ods << typeOf<std::string>()->signature();
      ods << future.error();
    } else {
      GenericValuePtr val = future.value();
      if (val.type->kind() != Type::Void)
        qi::details::serialize(val, ods);
      val.destroy();
    }
    ret.setBuffer(result);
    if (!socket->send(ret))
      qiLogError("qi.Server") << "Can't generate an answer for address:" << replyaddr;
  }
}

#endif  // _SRC_SERVERRESULT_HPP_

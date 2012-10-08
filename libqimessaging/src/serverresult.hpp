#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVERRESULT_HPP_
#define _SRC_SERVERRESULT_HPP_

#include <boost/shared_ptr.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/transportsocket.hpp>

namespace qi {

  inline void serverResultAdapter(qi::Future<GenericValue> future, TransportSocketPtr socket, const qi::MessageAddress &replyaddr) {
    qi::Message ret(Message::Type_Reply, replyaddr);
    qi::Buffer      result;
    qi::ODataStream ods(result);

    if (future.hasError()) {
      ret.setType(qi::Message::Type_Error);
      ods << typeOf<std::string>()->signature();
      ods << future.error();
    } else {
      GenericValue val = future.value();
      if (val.type->kind() != Type::Void)
        val.serialize(ods);
    }
    ret.setBuffer(result);
    if (!socket->send(ret))
      qiLogError("qi.Server") << "Can't generate an answer for address:" << replyaddr;
  }
}

#endif  // _SRC_SERVERRESULT_HPP_

#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVERRESULT_HPP_
#define _SRC_SERVERRESULT_HPP_

#include <boost/shared_ptr.hpp>
#include <qi/future.hpp>
#include "message.hpp"
#include "transportsocket.hpp"

namespace qi {

  inline void serverResultAdapter(qi::Future<GenericValuePtr> future,
    ObjectHost* host, TransportSocketPtr socket, const qi::MessageAddress &replyaddr) {
    qi::Message ret(Message::Type_Reply, replyaddr);

    if (future.hasError()) {
      ret.setType(qi::Message::Type_Error);
      ret.setError(future.error());
    } else {
      qi::GenericValuePtr val = future.value();
      ret.setValue(val, host);
      val.destroy();
    }
    if (!socket->send(ret))
      qiLogError("qimessaging.serverresult") << "Can't generate an answer for address:" << replyaddr;
  }
}

#endif  // _SRC_SERVERRESULT_HPP_

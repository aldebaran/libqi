#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVERRESULT_HPP_
#define _SRC_SERVERRESULT_HPP_

#include <qi/future.hpp>
#include "message.hpp"
#include "transportsocket.hpp"

namespace qi {

  // second bounce when returned type is a future
  inline void serverResultAdapterNext(GenericValuePtr val,// the future
    ObjectHost* host,
    TransportSocketPtr socket, const qi::MessageAddress &replyaddr)
  {
    qi::Message ret(Message::Type_Reply, replyaddr);
    TypeTemplate* futureType = QI_TEMPLATE_TYPE_GET(val.type, Future);
    ObjectTypeInterface* onext = dynamic_cast<ObjectTypeInterface*>(futureType->next());
    GenericObject gfut(onext, val.value);
    if (gfut.call<bool>("hasError", 0))
    {
      ret.setType(qi::Message::Type_Error);
      ret.setError(gfut.call<std::string>("error", 0));
    }
    else
    {
      GenericValue v = gfut.call<GenericValue>("value", 0);
      ret.setValue(v, host);
    }
    if (!socket->send(ret))
      qiLogError("qimessaging.serverresult") << "Can't generate an answer for address:" << replyaddr;
  }

  inline void serverResultAdapter(qi::Future<GenericValuePtr> future,
    ObjectHost* host, TransportSocketPtr socket, const qi::MessageAddress &replyaddr) {
    qi::Message ret(Message::Type_Reply, replyaddr);

    if (future.hasError()) {
      ret.setType(qi::Message::Type_Error);
      ret.setError(future.error());
    } else {
      qi::GenericValuePtr val = future.value();
      TypeTemplate* futureType = QI_TEMPLATE_TYPE_GET(val.type, Future);
      if (futureType)
      { // Return value is a future, bounce
        Type* next = futureType->next();
        ObjectTypeInterface* onext = dynamic_cast<ObjectTypeInterface*>(next);
        GenericObject gfut(onext, val.value);
        boost::function<void()> cb = boost::bind(serverResultAdapterNext, val, host, socket, replyaddr);
        gfut.call<void>("_connect", cb);
        return;
      }
      ret.setValue(val, host);
      val.destroy();
    }
    if (!socket->send(ret))
      qiLogError("qimessaging.serverresult") << "Can't generate an answer for address:" << replyaddr;
  }
}

#endif  // _SRC_SERVERRESULT_HPP_

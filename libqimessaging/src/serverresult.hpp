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
#include <qimessaging/metafunction.hpp>
#include <qimessaging/transportsocket.hpp>

namespace qi {

  inline void serverResultAdapter(qi::Future<MetaFunctionResult> future, TransportSocketPtr socket, const qi::MessageAddress &replyaddr) {
    qi::Message ret(replyaddr);

    if (future.hasError()) {
      qi::Buffer      result;
      qi::ODataStream ods(result);
      ods << future.error();
      ret.setBuffer(result);
      socket->send(ret);
      return;
    }
    ret.setBuffer(future.value().getBuffer());
    socket->send(ret);
  }
}

#endif  // _SRC_SERVERRESULT_HPP_

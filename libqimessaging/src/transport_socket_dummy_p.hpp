/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2012 Aldebaran Robotics
 */

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SOCKET_DUMMY_P_HPP_
#define _QIMESSAGING_TRANSPORT_SOCKET_DUMMY_P_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/session.hpp>
# include <qimessaging/message.hpp>
# include <qimessaging/transport_socket.hpp>
# include "src/transport_socket_p.hpp"

# include <string>
# include <queue>

namespace qi
{
  class TransportSocketDummyPrivate : public TransportSocketPrivate
  {
  public:
    TransportSocketDummyPrivate(TransportSocket *socket);
    virtual ~TransportSocketDummyPrivate();

    virtual bool connect(qi::Session *session,
                         const qi::Url &url);
    virtual void disconnect();
    virtual bool send(const qi::Message &msg);
  };
}

#endif  // _QIMESSAGING_TRANSPORT_SOCKET_DUMMY_P_HPP_

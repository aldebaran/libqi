#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVERCLIENT_HPP_
#define _SRC_SERVERCLIENT_HPP_

#include <qimessaging/future.hpp>
#include <qimessaging/genericobject.hpp>
#include "remoteobject_p.hpp"

namespace qi {

  class TransportSocket;
  class ServerClient {
  public:
    ServerClient(TransportSocketPtr socket);
    ~ServerClient();
    qi::Future<unsigned int>   registerEvent(unsigned int serviceId, unsigned eventId, unsigned int linkId);
    qi::Future<bool>           unregisterEvent(unsigned int serviceId, unsigned eventId, unsigned linkId);
    qi::Future<qi::MetaObject> metaObject(unsigned int serviceId, unsigned int objectId);

  public:
    qi::RemoteObject _remoteObject;
    qi::ObjectPtr    _object;
  };
}

#endif  // _SRC_SERVERCLIENT_HPP_

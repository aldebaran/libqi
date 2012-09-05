/*
**  Author(s):
**  - Cedric Gestes <gestes@aldebaran-robotics.com>
**
**  Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef    SERVER_CLIENT_HPP_
# define    SERVER_CLIENT_HPP_

#include <qimessaging/future.hpp>
#include <qimessaging/object.hpp>
#include "remoteobject_p.hpp"

namespace qi {

  class TransportSocket;
  class ServerClient {
  public:
    ServerClient(qi::TransportSocket *socket);
    ~ServerClient();
    qi::Future<unsigned int>   registerEvent(unsigned int serviceId, unsigned eventId, unsigned int linkId);
    qi::Future<bool>           unregisterEvent(unsigned int serviceId, unsigned eventId, unsigned linkId);
    qi::Future<qi::MetaObject> metaObject(unsigned int serviceId, unsigned int objectId);

  private:
    qi::TransportSocket *_socket;

  public:
    qi::RemoteObject     _object;
  };
}

#endif     /* !SERVER_CLIENT_PP_ */

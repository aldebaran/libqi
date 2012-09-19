/*
**  Author(s):
**  - Cedric Gestes <gestes@aldebaran-robotics.com>
**
**  Copyright (C) 2012 Aldebaran Robotics
*/

#include "server_client.hpp"
#include <qimessaging/message.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/objecttypebuilder.hpp>


namespace qi {

  static qi::MetaObject serverMetaObject() {
    qi::ObjectTypeBuilder<ServerClient>  ob;
    ObjectTypeBuilderBase::SignalMemberGetter dummy;

    ob.advertiseMethod("registerEvent", &ServerClient::registerEvent);
    ob.advertiseMethod("unregisterEvent", &ServerClient::unregisterEvent);
    ob.advertiseMethod("metaObject", &ServerClient::metaObject);


    qi::MetaObject m = ob.metaObject();
    assert(m.methodId("registerEvent::(III)") == qi::Message::ServerFunction_RegisterEvent);
    assert(m.methodId("unregisterEvent::(III)") == qi::Message::ServerFunction_UnregisterEvent);
    assert(m.methodId("metaObject::(II)") == qi::Message::ServerFunction_MetaObject);
    return m;
  }

  ServerClient::ServerClient(TransportSocketPtr socket)
    : _remoteObject(socket, qi::Message::Service_Server, serverMetaObject())
  {
    _object = makeDynamicObject(&_remoteObject);
  }

  ServerClient::~ServerClient()
  {
  }

  qi::Future<unsigned int> ServerClient::registerEvent(unsigned int serviceId, unsigned eventId, unsigned int linkId) {
    return _object.call<unsigned int>("registerEvent", serviceId, eventId, linkId);
  }

  qi::Future<bool> ServerClient::unregisterEvent(unsigned int serviceId, unsigned eventId, unsigned linkId) {
    return _object.call<bool>("unregisterEvent", serviceId, eventId, linkId);
  }

  qi::Future<qi::MetaObject> ServerClient::metaObject(unsigned int serviceId, unsigned int objectId) {
    return _object.call<qi::MetaObject>("metaObject", serviceId, objectId);
  };

}

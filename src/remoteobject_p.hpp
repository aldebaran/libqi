#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_REMOTEOBJECT_P_HPP_
#define _SRC_REMOTEOBJECT_P_HPP_

#include "transportsocket.hpp"
#include <qitype/genericobject.hpp>
#include <qitype/dynamicobject.hpp>
#include <qitype/signal.hpp>

#include "messagedispatcher.hpp"
#include "objecthost.hpp"

#include <boost/thread/mutex.hpp>
#include <string>

namespace qi {

  class TransportSocket;
  class ServerClient;


  class RemoteObject : public qi::DynamicObject, public ObjectHost {
  public:
    RemoteObject();
    RemoteObject(unsigned int service, qi::TransportSocketPtr socket = qi::TransportSocketPtr());
    //deprecated
    RemoteObject(unsigned int service, unsigned int object, qi::MetaObject metaObject, qi::TransportSocketPtr socket = qi::TransportSocketPtr());
    ~RemoteObject();

    //must be called to make the object valid.
    qi::Future<void> fetchMetaObject();

    void setTransportSocket(qi::TransportSocketPtr socket);
    void close();
    unsigned int service() const { return _service;}

  protected:
    //TransportSocket.messagePending
    void onMessagePending(const qi::Message &msg);
    //TransportSocket.disconnected
    void onSocketDisconnected(std::string error);

    virtual void metaPost(Manageable* context, unsigned int event, const GenericFunctionParameters& args);
    virtual qi::Future<AnyReference> metaCall(Manageable* context, unsigned int method, const GenericFunctionParameters& args, qi::MetaCallType callType = qi::MetaCallType_Auto);

    //metaObject received
    void onMetaObject(qi::Future<qi::MetaObject> fut, qi::Promise<void> prom);

    virtual qi::Future<Link> metaConnect(unsigned int event, const SignalSubscriber& sub);
    virtual qi::Future<void> metaDisconnect(Link linkId);

    virtual qi::Future<GenericValue> metaProperty(unsigned int id);
    virtual qi::Future<void> metaSetProperty(unsigned int id, GenericValue val);

  protected:
    TransportSocketPtr                              _socket;
    unsigned int                                    _service;
    unsigned int                                    _object;
    std::map<int, qi::Promise<AnyReference> > _promises;
    boost::mutex    _mutex;
    qi::SignalBase::Link                            _linkMessageDispatcher;
    qi::SignalBase::Link                            _linkDisconnected;
    qi::ObjectPtr                                   _self;
  };

}



#endif  // _SRC_REMOTEOBJECT_P_HPP_

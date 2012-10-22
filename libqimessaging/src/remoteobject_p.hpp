#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_REMOTEOBJECT_P_HPP_
#define _SRC_REMOTEOBJECT_P_HPP_

#include <qimessaging/datastream.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/dynamicobject.hpp>
#include <qitype/signal.hpp>

#include "messagedispatcher.hpp"

#include <boost/thread/mutex.hpp>
#include <string>

namespace qi {

  class TransportSocket;
  class ServerClient;


  class RemoteObject : public qi::DynamicObject {
  public:
    RemoteObject();
    RemoteObject(unsigned int service, qi::TransportSocketPtr socket);
    //deprecated
    RemoteObject(unsigned int service, qi::MetaObject metaObject, qi::TransportSocketPtr socket = qi::TransportSocketPtr());
    ~RemoteObject();

    //must be called to make the object valid.
    qi::Future<void> fetchMetaObject();

    void setTransportSocket(qi::TransportSocketPtr socket);
    void close();

  protected:
    //TransportSocket.messagePending
    void onMessagePending(const qi::Message &msg);
    //TransportSocket.disconnected
    void onSocketDisconnected(int error);

    virtual void metaEmit(unsigned int event, const GenericFunctionParameters& args);
    virtual qi::Future<GenericValue> metaCall(unsigned int method, const GenericFunctionParameters& args, qi::MetaCallType callType = qi::MetaCallType_Auto);

    //metaObject received
    void onMetaObject(qi::Future<qi::MetaObject> fut, qi::Promise<void> prom);

    virtual qi::Future<unsigned int> metaConnect(unsigned int event, const SignalSubscriber& sub);
    virtual qi::Future<void> metaDisconnect(unsigned int linkId);

  protected:
    TransportSocketPtr                              _socket;
    unsigned int                                    _service;
    std::map<int, qi::Promise<GenericValue> > _promises;
    boost::mutex    _mutex;
    qi::SignalBase::Link                            _linkMessageDispatcher;
    qi::SignalBase::Link                            _linkDisconnected;
    qi::ObjectPtr                                   _self;
  };

}



#endif  // _SRC_REMOTEOBJECT_P_HPP_

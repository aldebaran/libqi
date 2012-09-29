#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_REMOTEOBJECT_P_HPP_
#define _SRC_REMOTEOBJECT_P_HPP_

#include <qimessaging/datastream.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/dynamicobject.hpp>
#include <qimessaging/signal.hpp>

#include "messagedispatcher.hpp"
#include "object_p.hpp"

#include <boost/thread/mutex.hpp>
#include <string>

namespace qi {

  class TransportSocket;
  class ServerClient;


  class RemoteObject : public qi::DynamicObject {
  public:
    RemoteObject();
    RemoteObject(unsigned int service, qi::MetaObject metaObject, qi::TransportSocketPtr socket = qi::TransportSocketPtr());
    ~RemoteObject();

    void setTransportSocket(qi::TransportSocketPtr socket);
    void close();

  protected:
    //MessageDispatcher callback
    void onMessagePending(const qi::Message &msg);

    virtual void metaEmit(unsigned int event, const GenericFunctionParameters& args);
    virtual qi::Future<GenericValue> metaCall(unsigned int method, const GenericFunctionParameters& args, qi::MetaCallType callType = qi::MetaCallType_Auto);

    virtual qi::Future<unsigned int> connect(unsigned int event, const SignalSubscriber& sub);
    virtual qi::Future<void> disconnect(unsigned int linkId);

  protected:
    TransportSocketPtr                              _socket;
    unsigned int                                    _service;
    std::map<int, qi::Promise<GenericValue> > _promises;
    boost::mutex    _mutex;
    qi::SignalBase::Link                            _linkMessageDispatcher;
    qi::ServerClient                               *_serverClient;
  };

}



#endif  // _SRC_REMOTEOBJECT_P_HPP_

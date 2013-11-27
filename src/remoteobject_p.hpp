#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_REMOTEOBJECT_P_HPP_
#define _SRC_REMOTEOBJECT_P_HPP_

#include "transportsocket.hpp"
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobject.hpp>
#include <qitype/signal.hpp>

#include "messagedispatcher.hpp"
#include "objecthost.hpp"

#include <boost/thread/mutex.hpp>
#include <string>

namespace qi {

  class TransportSocket;
  class ServerClient;

  struct RemoteSignalLinks {
    RemoteSignalLinks()
      : remoteSignalLink(qi::SignalBase::invalidSignalLink)
    {}

    std::vector<qi::SignalLink> localSignalLink;
    qi::SignalLink              remoteSignalLink;
    qi::Future<qi::SignalLink>  future;
  };

  class RemoteObject : public qi::DynamicObject, public ObjectHost, public Trackable<RemoteObject> {
  public:
    RemoteObject();
    RemoteObject(unsigned int service, qi::TransportSocketPtr socket = qi::TransportSocketPtr());
    //deprecated
    RemoteObject(unsigned int service, unsigned int object, qi::MetaObject metaObject, qi::TransportSocketPtr socket = qi::TransportSocketPtr());
    ~RemoteObject();

    //must be called to make the object valid.
    qi::Future<void> fetchMetaObject();

    void setTransportSocket(qi::TransportSocketPtr socket);
    // Set fromSignal if close is invoked from disconnect signal callback
    void close(bool fromSignal = false);
    unsigned int service() const { return _service;}

  protected:
    //TransportSocket.messagePending
    void onMessagePending(const qi::Message &msg);
    //TransportSocket.disconnected
    void onSocketDisconnected(std::string error);

    virtual void metaPost(AnyObject context, unsigned int event, const GenericFunctionParameters& args);
    virtual qi::Future<AnyReference> metaCall(AnyObject context, unsigned int method, const GenericFunctionParameters& args, qi::MetaCallType callType = qi::MetaCallType_Auto);

    //metaObject received
    void onMetaObject(qi::Future<qi::MetaObject> fut, qi::Promise<void> prom);

    virtual qi::Future<SignalLink> metaConnect(unsigned int event, const SignalSubscriber& sub);
    virtual qi::Future<void> metaDisconnect(SignalLink linkId);

    virtual qi::Future<AnyValue> metaProperty(unsigned int id);
    virtual qi::Future<void> metaSetProperty(unsigned int id, AnyValue val);

  protected:
    typedef std::map<qi::uint64_t, RemoteSignalLinks>  LocalToRemoteSignalLinkMap;

    TransportSocketPtr                              _socket;
    boost::mutex                                    _socketMutex;
    unsigned int                                    _service;
    unsigned int                                    _object;
    std::map<int, qi::Promise<AnyReference> >       _promises;
    boost::mutex                                    _promisesMutex;
    qi::SignalLink                                  _linkMessageDispatcher;
    qi::SignalLink                                  _linkDisconnected;
    qi::AnyObject                                   _self;

    boost::recursive_mutex                          _localToRemoteSignalLinkMutex;
    LocalToRemoteSignalLinkMap                      _localToRemoteSignalLink;
  };

}



#endif  // _SRC_REMOTEOBJECT_P_HPP_

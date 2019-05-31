#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_REMOTEOBJECT_P_HPP_
#define _SRC_REMOTEOBJECT_P_HPP_

#include "messagesocket.hpp"
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobject.hpp>
#include <qi/signal.hpp>

#include "messagedispatcher.hpp"
#include "objecthost.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <string>

namespace qi {

  class MessageSocket;
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
    RemoteObject(unsigned int service, qi::MessageSocketPtr socket = qi::MessageSocketPtr(),
      boost::optional<ObjectUid> uid = boost::none);
    //deprecated
    RemoteObject(unsigned int service, unsigned int object, qi::MetaObject metaObject, qi::MessageSocketPtr socket = qi::MessageSocketPtr());
    ~RemoteObject();

    unsigned int nextId() { return ++_nextId; }

    //must be called to make the object valid.
    qi::Future<void> fetchMetaObject();

    void setTransportSocket(qi::MessageSocketPtr socket);
    // Set fromSignal if close is invoked from disconnect signal callback
    void close(const std::string& reason, bool fromSignal = false);
    unsigned int service() const { return _service; }
    unsigned int object() const { return _object; }

  protected:
    //TransportSocket.messagePending
    void onMessagePending(const qi::Message &msg);
    //TransportSocket.disconnected
    void onSocketDisconnected(std::string error);

    virtual void metaPost(AnyObject context, unsigned int event, const GenericFunctionParameters& args);
    virtual qi::Future<AnyReference> metaCall(AnyObject context, unsigned int method, const GenericFunctionParameters& args, qi::MetaCallType callType, Signature returnSignature);
    void onFutureCancelled(unsigned int originalMessageId);

    //metaObject received
    void onMetaObject(qi::Future<qi::MetaObject> fut, qi::Promise<void> prom);

    virtual qi::Future<SignalLink> metaConnect(unsigned int event, const SignalSubscriber& sub);
    virtual qi::Future<void> metaDisconnect(SignalLink linkId);

    virtual qi::Future<AnyValue> metaProperty(qi::AnyObject context, unsigned int id);
    virtual qi::Future<void> metaSetProperty(qi::AnyObject context, unsigned int id, AnyValue val);

  protected:
    using LocalToRemoteSignalLinkMap = std::map<qi::uint64_t, RemoteSignalLinks>;

    boost::synchronized_value<MessageSocketPtr>   _socket;
    unsigned int                                    _service;
    unsigned int                                    _object;
    boost::synchronized_value<std::map<int, qi::Promise<AnyReference>>> _promises;
    qi::SignalLink                                  _linkMessageDispatcher;
    qi::SignalLink                                  _linkDisconnected;
    qi::AnyObject                                   _self;
    boost::recursive_mutex                          _localToRemoteSignalLinkMutex;
    LocalToRemoteSignalLinkMap                      _localToRemoteSignalLink;

  private:
    static qi::Atomic<unsigned int> _nextId;
  };

  using RemoteObjectPtr = boost::shared_ptr<RemoteObject>;

}



#endif  // _SRC_REMOTEOBJECT_P_HPP_

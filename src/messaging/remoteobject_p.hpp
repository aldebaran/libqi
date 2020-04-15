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
#include <qi/messaging/messagesocket_fwd.hpp>

#include "messagedispatcher.hpp"
#include "objecthost.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <string>

namespace qi {

  class ServerClient;

  struct RemoteSignalLinks
  {
    std::vector<qi::SignalLink> localSignalLink;
    qi::SignalLink remoteSignalLink = SignalBase::invalidSignalLink;
    qi::Future<qi::SignalLink>  future;
  };

  class RemoteObject;
  using RemoteObjectPtr = boost::shared_ptr<RemoteObject>;

  /// This class represents the interface to a remote (potentially in a different process or host)
  /// `qi::Object`.
  ///
  /// Remote objects are proxies of the concrete objects and are responsible for transmitting them
  /// requests (and receiving responses).
  ///
  /// Remote objects are created when:
  ///   - A user requests the object representing a service (those live in the user's process).
  ///   - An object is received as the result of an outgoing remote procedure call (those live in
  /// the process sending the call).
  ///   - An object is received as a parameter of an incoming procedure call (those live in
  /// the process receiving the call).
  class RemoteObject
    : public DynamicObject
    , public ObjectHost
    , public boost::enable_shared_from_this<RemoteObject>
  {
    RemoteObject(unsigned int service, unsigned int object = Message::GenericObject_Main,
                 boost::optional<ObjectUid> uid = boost::none);

  public:
    template<typename... Args>
    static RemoteObjectPtr makePtr(Args&&... args)
    {
      return RemoteObjectPtr(new RemoteObject(std::forward<Args>(args)...));
    }

    ~RemoteObject() override;

    unsigned int nextId() override { return ++_nextId; }

    //must be called to make the object valid.
    qi::Future<void> fetchMetaObject();

    void setTransportSocket(qi::MessageSocketPtr socket);
    // Set fromSignal if close is invoked from disconnect signal callback
    void close(const std::string& reason, bool fromSignal = false);
    unsigned int service() const { return _service; }
    unsigned int object() const { return _object; }

    void metaPost(AnyObject context,
                  unsigned int event,
                  const GenericFunctionParameters& args) override;
    qi::Future<AnyReference> metaCall(AnyObject context,
                                      unsigned int method,
                                      const GenericFunctionParameters& args,
                                      qi::MetaCallType callType = MetaCallType_Auto,
                                      Signature returnSignature = {}) override;

    qi::Future<SignalLink> metaConnect(unsigned int event, const SignalSubscriber& sub) override;
    qi::Future<void> metaDisconnect(SignalLink linkId) override;

    qi::Future<AnyValue> metaProperty(qi::AnyObject context, unsigned int id) override;
    qi::Future<void> metaSetProperty(qi::AnyObject context, unsigned int id, AnyValue val) override;

  protected:
    //TransportSocket.messagePending
    DispatchStatus onMessagePending(const qi::Message &msg);
    //TransportSocket.disconnected
    void onSocketDisconnected(std::string error);

    void onFutureCancelled(unsigned int originalMessageId);

    //metaObject received
    void onMetaObject(qi::Future<qi::MetaObject> fut, qi::Promise<void> prom);

  protected:
    using LocalToRemoteSignalLinkMap = std::map<qi::uint64_t, RemoteSignalLinks>;

    boost::synchronized_value<MessageSocketPtr>   _socket;
    unsigned int                                    _service;
    unsigned int                                    _object;
    boost::synchronized_value<std::map<int, qi::Promise<AnyReference>>> _promises;
    qi::SignalLink _linkMessageDispatcher = SignalBase::invalidSignalLink;
    qi::SignalLink _linkDisconnected = SignalBase::invalidSignalLink;
    qi::AnyObject                                   _self;
    boost::recursive_mutex                          _localToRemoteSignalLinkMutex;
    LocalToRemoteSignalLinkMap                      _localToRemoteSignalLink;

  private:
    static qi::Atomic<unsigned int> _nextId;
  };

}



#endif  // _SRC_REMOTEOBJECT_P_HPP_

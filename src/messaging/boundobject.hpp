#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BOUNDOBJECT_HPP_
#define _SRC_BOUNDOBJECT_HPP_

#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/signals2.hpp>
#include <boost/optional.hpp>
#include <qi/api.hpp>
#include <qi/session.hpp>
#include "transportserver.hpp"
#include <qi/atomic.hpp>
#include <qi/strand.hpp>

#include "objecthost.hpp"

using AtomicBoolptr = boost::shared_ptr<qi::Atomic<bool>>;
using AtomicIntPtr = boost::shared_ptr<qi::Atomic<int>>;

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;
  class ServiceDirectory;

  // (service, linkId)
  struct RemoteSignalLink
  {
    RemoteSignalLink()
      : localSignalLinkId()
      , event(0)
    {}

    RemoteSignalLink(qi::Future<SignalLink> localSignalLinkId, unsigned int event)
    : localSignalLinkId(localSignalLinkId)
    , event(event) {}

    qi::Future<SignalLink> localSignalLinkId;
    unsigned int event;
  };


  class BoundObject {
  public:
    //Server Interface
    virtual ~BoundObject() {}
    virtual void onMessage(const qi::Message &msg, TransportSocketPtr socket) = 0;
    virtual void onSocketDisconnected(qi::TransportSocketPtr socket, std::string error) = 0;
  };

  //Bound Object, represent an object bound on a server
  // this is not an object..
  class ServiceBoundObject
    : public BoundObject
    , public ObjectHost
    , boost::noncopyable
    , public Trackable<ServiceBoundObject> {

  public:
    // TODO: once VS2013 build farm is fixed, replaced the following constructors by this one.
    //ServiceBoundObject(unsigned int serviceId, unsigned int objectId,
                       //qi::AnyObject obj,
                       //qi::MetaCallType mct = qi::MetaCallType_Queued,
                       //bool bindTerminate = false,
                       //boost::optional<boost::weak_ptr<ObjectHost>> owner = {});

    ServiceBoundObject(unsigned int serviceId, unsigned int objectId,
                       qi::AnyObject obj,
                       qi::MetaCallType mct,
                       bool bindTerminatee,
                       boost::optional<boost::weak_ptr<ObjectHost>> owner);

    ServiceBoundObject(unsigned int serviceId, unsigned int objectId,
                      qi::AnyObject obj,
                      qi::MetaCallType mct = qi::MetaCallType_Queued,
                      bool bindTerminate = false)
      : ServiceBoundObject(serviceId, objectId, obj, mct, bindTerminate, {})
    {}

    virtual ~ServiceBoundObject();

    unsigned int nextId() { return ++_nextId; }

  public:
    //PUBLIC BOUND METHODS
    qi::Future<SignalLink> registerEvent(unsigned int serviceId, unsigned int eventId, SignalLink linkId);
    qi::Future<SignalLink> registerEventWithSignature(unsigned int serviceId, unsigned int eventId, SignalLink linkId, const std::string& signature);
    qi::Future<void> unregisterEvent(unsigned int serviceId, unsigned int eventId, SignalLink linkId);
    qi::MetaObject metaObject(unsigned int serviceId);
    void           terminate(unsigned int serviceId); //bound only in special cases
    qi::Future<AnyValue> property(const AnyValue& name);
    Future<void>   setProperty(const AnyValue& name, AnyValue value);
    std::vector<std::string> properties();
  public:
    /*
    * Returns the last socket that sent a message to this object.
    * Considering the volatility of this operation, Users of currentSocket()
    * must set _callType to Direct, otherwise behavior is undefined. Calling
    * currentSocket multiple times in a row in the same context should be
    * avoided: call it once and use the return value, unless you know what
    * you're doing.
    */
    inline qi::TransportSocketPtr currentSocket() const {
#ifndef NDEBUG
      if (_callType != MetaCallType_Direct)
        qiLogWarning("qimessaging.boundobject") << " currentSocket() used but callType is not direct";
#endif
      boost::recursive_mutex::scoped_lock lock(_mutex);
      return _currentSocket;
    }

    inline AnyObject object() { return _object;}
  public:
    //BoundObject Interface
    virtual void onMessage(const qi::Message &msg, TransportSocketPtr socket);
    virtual void onSocketDisconnected(qi::TransportSocketPtr socket, std::string error);

    using MessageId = unsigned int;
    void cancelCall(TransportSocketPtr origSocket, const Message& cancelMessage, MessageId origMsgId);

  private:
    using FutureMap = std::map<MessageId, std::pair<Future<AnyReference>, AtomicIntPtr>>;
    using CancelableMap = std::map<TransportSocketPtr, FutureMap>;
    struct CancelableKit;
    using CancelableKitPtr = boost::shared_ptr<CancelableKit>;
    CancelableKitPtr _cancelables;
    using CancelableKitWeak = boost::weak_ptr<CancelableKit>;

    qi::AnyObject createServiceBoundObjectType(ServiceBoundObject *self, bool bindTerminate = false);

    inline boost::weak_ptr<ObjectHost> _gethost() { return _owner ? *_owner : weakPtr(); }
    static void _removeCachedFuture(CancelableKitWeak kit, TransportSocketPtr sock, MessageId id);
    static void serverResultAdapterNext(AnyReference val, Signature targetSignature,
                                        boost::weak_ptr<ObjectHost> host,
                                 TransportSocketPtr sock, const MessageAddress& replyAddr,
                                 const Signature& forcedReturnSignature, CancelableKitWeak kit);
    static void serverResultAdapter(Future<AnyReference> future, const Signature& targetSignature,
                                    boost::weak_ptr<ObjectHost> host,
                                    TransportSocketPtr sock, const MessageAddress& replyAddr,
                                    const Signature& forcedReturnSignature, CancelableKitWeak kit,
                                    AtomicIntPtr cancelRequested = AtomicIntPtr());

  private:
    // remote link id -> local link id
    using ServiceSignalLinks = std::map<SignalLink, RemoteSignalLink>;
    using BySocketServiceSignalLinks = std::map<qi::TransportSocketPtr, ServiceSignalLinks>;

    //Event handling (no lock needed)
    BySocketServiceSignalLinks  _links;

    boost::mutex _callMutex;
  private:
    qi::TransportSocketPtr _currentSocket;
    unsigned int           _serviceId;
    unsigned int           _objectId;
    qi::AnyObject          _object;
    qi::AnyObject          _self;
    qi::MetaCallType       _callType;
    boost::optional<boost::weak_ptr<qi::ObjectHost>> _owner;
    // prevents parallel onMessage on self execution and protects the current socket
    mutable boost::recursive_mutex           _mutex;
    boost::function<void (TransportSocketPtr, std::string)> _onSocketDisconnectedCallback;

    static qi::Atomic<unsigned int> _nextId;

    friend class ::qi::ObjectHost;
    friend class ::qi::ServiceDirectory;
  };

  using BoundAnyObject = boost::shared_ptr<BoundObject>;

  qi::BoundAnyObject makeServiceBoundAnyObject(unsigned int serviceId, qi::AnyObject object, qi::MetaCallType mct = qi::MetaCallType_Auto);

}

#endif  // _SRC_BOUNDOBJECT_HPP_

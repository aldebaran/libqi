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
#include <qi/api.hpp>
#include <qi/session.hpp>
#include "transportserver.hpp"
#include <qi/atomic.hpp>
#include <qi/strand.hpp>

#include "objecthost.hpp"

typedef boost::shared_ptr<qi::Atomic<bool> > AtomicBoolptr;
typedef boost::shared_ptr<qi::Atomic<int> > AtomicIntPtr;

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;
  class ServiceDirectory;

  // (service, linkId)
  struct RemoteSignalLink
  {
    RemoteSignalLink()
      : localSignalLinkId(0)
      , event(0)
    {}
    RemoteSignalLink(SignalLink localSignalLinkId, unsigned int event)
    : localSignalLinkId(localSignalLinkId)
    , event(event) {}
    SignalLink localSignalLinkId;
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
  class ServiceBoundObject : public BoundObject, public ObjectHost, boost::noncopyable {

  public:
    ServiceBoundObject(unsigned int serviceId, unsigned int objectId,
                       qi::AnyObject obj,
                       qi::MetaCallType mct = qi::MetaCallType_Queued,
                       bool bindTerminate = false,
                       ObjectHost* owner = 0);
    virtual ~ServiceBoundObject();

  public:
    //PUBLIC BOUND METHODS
    SignalLink           registerEvent(unsigned int serviceId, unsigned int eventId, SignalLink linkId);
    SignalLink           registerEventWithSignature(unsigned int serviceId, unsigned int eventId, SignalLink linkId, const std::string& signature);
    void           unregisterEvent(unsigned int serviceId, unsigned int eventId, SignalLink linkId);
    qi::MetaObject metaObject(unsigned int serviceId);
    void           terminate(unsigned int serviceId); //bound only in special cases
    qi::AnyValue   property(const AnyValue& name);
    void           setProperty(const AnyValue& name, AnyValue value);
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

    typedef unsigned int MessageId;
    void cancelCall(TransportSocketPtr origSocket, const Message& cancelMessage, MessageId origMsgId);

    qi::Signal<ServiceBoundObject*> onDestroy;
  private:
    typedef std::map<MessageId, std::pair<Future<AnyReference>, AtomicIntPtr> > FutureMap;
    typedef std::map<TransportSocketPtr, FutureMap> CancelableMap;
    struct CancelableKit;
    typedef boost::shared_ptr<CancelableKit> CancelableKitPtr;
    CancelableKitPtr _cancelables;
    typedef boost::weak_ptr<CancelableKit> CancelableKitWeak;

    qi::AnyObject createServiceBoundObjectType(ServiceBoundObject *self, bool bindTerminate = false);


    inline ObjectHost* _gethost() { return _owner ? _owner : this; }
    static void _removeCachedFuture(CancelableKitWeak kit, TransportSocketPtr sock, MessageId id);
    static void serverResultAdapterNext(AnyReference val, Signature targetSignature, ObjectHost* host,
                                 TransportSocketPtr sock, const MessageAddress& replyAddr,
                                 const Signature& forcedReturnSignature, CancelableKitWeak kit);
    static void serverResultAdapter(Future<AnyReference> future, const Signature& targetSignature, ObjectHost* host,
                                    TransportSocketPtr sock, const MessageAddress& replyAddr,
                                    const Signature& forcedReturnSignature, CancelableKitWeak kit,
                                    AtomicIntPtr cancelRequested = AtomicIntPtr());

  private:
    // remote link id -> local link id
    typedef std::map<SignalLink, RemoteSignalLink>             ServiceSignalLinks;
    typedef std::map<qi::TransportSocketPtr, ServiceSignalLinks> BySocketServiceSignalLinks;

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
    qi::ObjectHost*        _owner;
    // prevents parallel onMessage on self execution and protects the current socket
    mutable boost::recursive_mutex           _mutex;
    boost::function<void (TransportSocketPtr, std::string)> _onSocketDisconnectedCallback;
    friend class ::qi::ObjectHost;
    friend class ::qi::ServiceDirectory;
  };


  typedef boost::shared_ptr<BoundObject> BoundAnyObject;

  qi::BoundAnyObject makeServiceBoundAnyObject(unsigned int serviceId, qi::AnyObject object, qi::MetaCallType mct = qi::MetaCallType_Auto);

}

#endif  // _SRC_BOUNDOBJECT_HPP_
